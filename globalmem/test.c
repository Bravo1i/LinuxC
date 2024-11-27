#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define GLOBALMEM_MAGIC 'g'
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)

FILE *file = NULL;
void *write_to_file(void *thread_data) {
    char *data = (char *)thread_data;
 
    // 写入文件
    if (file == NULL) {
        file = fopen("/dev/globalmem", "rw+");
        if (file == NULL) {
            perror("Error opening file");
            return NULL;
        }
    }
    int ret = fprintf(file, "%s\n", data);
    printf("%s, ret:%d\n", data, ret);
    fflush(file); // 立即将数据写入硬盘

    free(data); // 释放线程数据
    return NULL;
}
 
int main(void)
{
    pthread_t threads[10];
    char *messages[10] = {"Thread 1", "Thread 2", "Thread 3", "Thread 4", "Thread 5",
                        "Thread 6", "Thread 7", "Thread 8", "Thread 9", "Thread 0"};
    
    for (int i = 0; i < 10; i++) {
        char *message = strdup(messages[i]); // 创建数据的副本
        if (message == NULL) {
            perror("Error creating message");
            return 1;
        }
        if (pthread_create(&threads[i], NULL, &write_to_file, (void *)message) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }
 
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
 
    if (file != NULL) {
        fclose(file);
    }
 
    return 0;
}
