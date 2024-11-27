#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define GLOBALMEM_SIZE 0x1000
// #define MEM_CLEAR 0x1
// 使用幻数定义cmd
#define GLOBALMEM_MAGIC 'g'
#define MEM_CLEAR _IO(GLOBALMEM_MAGIC, 0)
#define GLOBALMEM_MAJOR 230
#define DEVICE_NUM 10

static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);

struct globalmem_dev {
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
    struct mutex mutex;
};

struct globalmem_dev *globalmem_devp;

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    // ppos 是要读的位置相对于文件开头的偏移
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data;

    if (p >= GLOBALMEM_SIZE)
        return 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;
    // 超过区间或者大于可写范围
    mutex_lock(&dev->mutex);
    if (copy_to_user(buf, dev->mem + p, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;

        printk(KERN_ERR"read %u bytes from %lu\n", count, p);
    }
    mutex_unlock(&dev->mutex);
    return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data;

    if (p >= GLOBALMEM_SIZE)
        return 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;
    // 超过区间或者大于可写范围
    mutex_lock(&dev->mutex);
    if (copy_from_user(dev->mem + p, buf, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;

        printk(KERN_ERR"written %u bytes from %lu\n", count, p);
    }
    mutex_unlock(&dev->mutex);
    return ret;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret = 0;
    switch (orig) {
    case 0:
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
        if ((unsigned int)offset > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int)offset;
        ret = filp->f_pos;
        break;
    case 1:
        if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        if ((filp->f_pos + offset) < 0) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos += offset;
        ret = filp->f_pos;
        break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;
    switch (cmd) {
    case MEM_CLEAR:
        mutex_lock(&dev->mutex);
        memset(dev->mem, 0, GLOBALMEM_SIZE);
        mutex_unlock(&dev->mutex);
        printk(KERN_ERR"globalmem is set to zero\n");
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static int globalmem_open(struct inode *inode, struct file *filp)
{
    // filp->private_data = globalmem_devp;
    // 文件私有数据指向设备结构体

    struct globalmem_dev *dev = container_of(inode->i_cdev, struct globalmem_dev, cdev);
    // 结构体成员的指针 整个结构体类型 结构体成员类型
    filp->private_data = dev;
    return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations globalmem_fops = {
    .owner = THIS_MODULE,
    .llseek = globalmem_llseek,
    .read = globalmem_read,
    .write = globalmem_write,
    .unlocked_ioctl = globalmem_ioctl,
    .open = globalmem_open,
    .release = globalmem_release,
};
// 文件描述符

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int err, devno = MKDEV(globalmem_major, index);
    // 主设备号和副设备号
    cdev_init(&dev->cdev, &globalmem_fops);
    // 初始化字符设备
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_ERR"Error %d adding globalmem:%d", err, index);
}

static int __init globalmem_init(void)
{
    int ret;
    dev_t dev_no = MKDEV(globalmem_major, 0);

    if (globalmem_major)
        ret = register_chrdev_region(dev_no, DEVICE_NUM, "globalmem");
    else {
        ret = alloc_chrdev_region(&dev_no, 0, DEVICE_NUM, "globalmem");
        globalmem_major = MAJOR(dev_no);
    }
    if (ret < 0)
        return ret;
    // 申请设备号

    globalmem_devp = kzalloc(sizeof(struct globalmem_dev) * DEVICE_NUM, GFP_KERNEL);
    if (!globalmem_devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    for (int i = 0; i < DEVICE_NUM; i++) {
        mutex_init(&globalmem_devp->mutex);
        globalmem_setup_cdev(globalmem_devp + i, i);
    }

    return 0;

fail_malloc:
    unregister_chrdev_region(dev_no, 1);
    return ret;

}
module_init(globalmem_init);

static void __exit globalmem_exit(void)
{
    for (int i = 0; i < DEVICE_NUM; i++)
        cdev_del(&(globalmem_devp + i)->cdev);
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major, 0), DEVICE_NUM);
}
module_exit(globalmem_exit);

MODULE_AUTHOR("Zhaorui.li");
MODULE_LICENSE("GPL V2");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
