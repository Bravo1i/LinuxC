#include <linux/init.h>
#include <linux/module.h>

static char *book_name = "dissection Linux Device Driver";
module_param(book_name, charp, S_IRUGO);

static int book_num = 400;
module_param(book_num, int, S_IRUGO);
// 带参数的内核模块

static int hello_data __initdata = 1;
static int __init hello_init(void)
{
    printk(KERN_ERR"Hello init data:%d\n", hello_data);
    printk(KERN_ERR"Book name:%s\n", book_name);
    printk(KERN_ERR"Book num:%d\n", book_num);
    printk(KERN_ERR"Hello world enter\n");
    return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
    printk(KERN_ERR"Hello world  exit\n");
}
module_exit(hello_exit);

int add_integar(int a, int b)
{
    return a+b;
}
EXPORT_SYMBOL(add_integar);

int sub_integar(int a, int b)
{
    return a-b;
}
EXPORT_SYMBOL(sub_integar);
// 通过 grep integar /proc/kallsyms 可以在内核中查到函数


MODULE_AUTHOR("Zhaorui.li");
MODULE_LICENSE("GPL V2");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
