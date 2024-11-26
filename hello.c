#include <linux/init.h>
#include <linux/module.h>
static int __init hello_init(void)
{
    printk(KERN_ERR"Hello world enter\n");
    return 0;
}
module_init(hello_init);

static void __exit hello_exit(void)
{
    printk(KERN_ERR"Hello world  exit\n");
}
module_exit(hello_exit);

MODULE_AUTHOR("Zhaorui.li");
MODULE_LICENSE("GPL V2");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
