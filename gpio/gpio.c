#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DRIVER_NAME "my_driver"
#define DRIVER_CLASS "MyModule"

static dev_t my_device_nr; // so number
static struct class *my_class; //lop
static struct cdev my_device;


static int my_open(struct inode *inode, struct file *file) {
    printk("Device open was called\n");
    return 0;
}

static int my_close(struct inode *inode, struct file *file) {
    printk("Device close was called\n");
    return 0;
}

static ssize_t my_read(struct file *file, char *buffer, size_t size, loff_t *offs) {
    return (copy_to_user(buffer, "Hello user\n", 11) ? 0 : 11);
}

static ssize_t my_write(struct file *file, const char *buffer, size_t size, loff_t *offs) {
    char data[10];
    memset(data, 0, 10);
    if (copy_from_user(data, buffer, size))
        return 0;

    printk("User write: %s", data);
    return size;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write,
};

static int __init simple_init(void) {
    printk("Hello kernel\n");
    //dang ky device bat dau tu 0 va co 1 so bao gom major va minor
    if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0){
        printk ("Could not allocate device\n");
        return -1;
    }
   //in so major va minor, my_device_nr se chua thong tin nay.
   //no co 12 bit chua major va 20 bit chua minor
    printk("my_driver: major = %d ", my_device_nr >> 20);
    printk ("minor = %d",my_device_nr & 0xfffff);

   //tao lop
   if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
        printk ("Could not create class\n");
        unregister_chrdev_region(my_device_nr, 1);
        return -1;
   }

    //tao device file
    if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL){
        printk ("Could not create device file\n");
        class_destroy(my_class);
        return -1;
    }

    //map vs user
    cdev_init(&my_device, &fops);

    //add device
    if(cdev_add (&my_device, my_device_nr, 1) == 1){
        printk ("Could not add device\n");
        device_destroy(my_class, my_device_nr);
        return -1;
    }

    return 0;
}

static void __exit simple_exit(void) {
    printk("Goodbye kernel\n");
    cdev_del(&my_device);
    device_destroy(my_class, my_device_nr);
    class_destroy(my_class);
    unregister_chrdev_region(my_device_nr, 1);
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phan Hao");
MODULE_VERSION("1.0");
