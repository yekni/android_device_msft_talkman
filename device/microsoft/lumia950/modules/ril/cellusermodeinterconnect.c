#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/poll.h>

#define DEVICE_NAME "cellusermode"
#define CLASS_NAME "lumia_cell"
#define MAX_BUFFER_SIZE 4096

static int major;
static struct class *cell_class;
static struct device *cell_device;
static struct cdev cell_cdev;

struct cell_dev {
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
    struct mutex lock;
    unsigned char *read_buffer;
    unsigned char *write_buffer;
    size_t read_count;
    size_t write_count;
    bool device_open;
};

static struct cell_dev *cell_device_data;

/* File operations */
static int cell_open(struct inode *inode, struct file *file)
{
    if (cell_device_data->device_open)
        return -EBUSY;

    cell_device_data->device_open = true;
    file->private_data = cell_device_data;
    pr_info("CELLUSERMODE: Device opened\n");
    return 0;
}

static int cell_release(struct inode *inode, struct file *file)
{
    struct cell_dev *dev = file->private_data;
    dev->device_open = false;
    pr_info("CELLUSERMODE: Device closed\n");
    return 0;
}

static ssize_t cell_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    struct cell_dev *dev = file->private_data;
    ssize_t ret;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    while (dev->read_count == 0) {
        mutex_unlock(&dev->lock);
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        if (wait_event_interruptible(dev->read_queue, dev->read_count > 0))
            return -ERESTARTSYS;
        if (mutex_lock_interruptible(&dev->lock))
            return -ERESTARTSYS;
    }

    /* Copy data to user space */
    if (count > dev->read_count)
        count = dev->read_count;
    
    if (copy_to_user(buf, dev->read_buffer, count)) {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }

    /* Move remaining data to buffer start */
    if (count < dev->read_count)
        memmove(dev->read_buffer, dev->read_buffer + count, dev->read_count - count);
    
    dev->read_count -= count;
    ret = count;

    mutex_unlock(&dev->lock);
    return ret;
}

static ssize_t cell_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    struct cell_dev *dev = file->private_data;
    ssize_t ret;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    /* Check if there's space in the buffer */
    if (dev->write_count >= MAX_BUFFER_SIZE) {
        mutex_unlock(&dev->lock);
        if (file->f_flags & O_NONBLOCK)
            return -EAGAIN;
        if (wait_event_interruptible(dev->write_queue, dev->write_count < MAX_BUFFER_SIZE))
            return -ERESTARTSYS;
        if (mutex_lock_interruptible(&dev->lock))
            return -ERESTARTSYS;
    }

    /* Limit write to available space */
    if (count > (MAX_BUFFER_SIZE - dev->write_count))
        count = MAX_BUFFER_SIZE - dev->write_count;

    if (copy_from_user(dev->write_buffer + dev->write_count, buf, count)) {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }

    dev->write_count += count;
    ret = count;

    /* Signal that data is available */
    wake_up_interruptible(&dev->read_queue);

    mutex_unlock(&dev->lock);
    return ret;
}

static unsigned int cell_poll(struct file *file, poll_table *wait)
{
    struct cell_dev *dev = file->private_data;
    unsigned int mask = 0;

    poll_wait(file, &dev->read_queue, wait);
    poll_wait(file, &dev->write_queue, wait);

    if (dev->read_count > 0)
        mask |= POLLIN | POLLRDNORM;
    if (dev->write_count < MAX_BUFFER_SIZE)
        mask |= POLLOUT | POLLWRNORM;

    return mask;
}

static const struct file_operations cell_fops = {
    .owner = THIS_MODULE,
    .open = cell_open,
    .release = cell_release,
    .read = cell_read,
    .write = cell_write,
    .poll = cell_poll,
};

/* Module init and exit */
static int __init cellusermode_init(void)
{
    int ret;

    /* Allocate device data */
    cell_device_data = kzalloc(sizeof(struct cell_dev), GFP_KERNEL);
    if (!cell_device_data)
        return -ENOMEM;

    /* Allocate buffers */
    cell_device_data->read_buffer = kzalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    cell_device_data->write_buffer = kzalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    if (!cell_device_data->read_buffer || !cell_device_data->write_buffer) {
        ret = -ENOMEM;
        goto err_free_buffers;
    }

    /* Initialize synchronization primitives */
    mutex_init(&cell_device_data->lock);
    init_waitqueue_head(&cell_device_data->read_queue);
    init_waitqueue_head(&cell_device_data->write_queue);

    /* Register character device */
    ret = alloc_chrdev_region(&major, 0, 1, DEVICE_NAME);
    if (ret < 0)
        goto err_free_buffers;

    cell_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(cell_class)) {
        ret = PTR_ERR(cell_class);
        goto err_unreg_chrdev;
    }

    cell_device = device_create(cell_class, NULL, major, NULL, DEVICE_NAME);
    if (IS_ERR(cell_device)) {
        ret = PTR_ERR(cell_device);
        goto err_destroy_class;
    }

    cdev_init(&cell_cdev, &cell_fops);
    ret = cdev_add(&cell_cdev, major, 1);
    if (ret < 0)
        goto err_destroy_device;

    pr_info("CELLUSERMODE: Module initialized successfully\n");
    return 0;

err_destroy_device:
    device_destroy(cell_class, major);
err_destroy_class:
    class_destroy(cell_class);
err_unreg_chrdev:
    unregister_chrdev_region(major, 1);
err_free_buffers:
    kfree(cell_device_data->read_buffer);
    kfree(cell_device_data->write_buffer);
    kfree(cell_device_data);
    return ret;
}

static void __exit cellusermode_exit(void)
{
    cdev_del(&cell_cdev);
    device_destroy(cell_class, major);
    class_destroy(cell_class);
    unregister_chrdev_region(major, 1);
    kfree(cell_device_data->read_buffer);
    kfree(cell_device_data->write_buffer);
    kfree(cell_device_data);
    pr_info("CELLUSERMODE: Module removed\n");
}

module_init(cellusermode_init);
module_exit(cellusermode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Android4Lumia");
MODULE_DESCRIPTION("Microsoft Lumia Cell User Mode Interface Driver");
MODULE_VERSION("1.0"); 