#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // for copy_to_user, copy_from_user
#include <linux/vfs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>   // for kernel thread functions
#include <linux/sched.h>     // for kthread_should_stop()

#define DEVICE_PATH "/dev/stick-data"
#define BUFFER_SIZE 256
#define MODULE_NAME "stick-controler"
#define nullptr (void*)0

static struct task_struct *my_thread;

void klog(const char* action, const char* message);
char* read_from_device(const char* device_path);
static int __init my_module_init(void);
static void __exit my_module_exit(void);

void klog(const char* action, const char* message) {
    pr_info("[%s] %s: %s\n", MODULE_NAME, action, message);
}

char* read_from_device(const char* device_path) {
    static struct file *file = nullptr;
    char *buffer;
    ssize_t bytes_read;
    loff_t offset = 0;

    // allocate memory for the buffer to read data
    buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buffer) {
        klog("kmalloc", "allocation failed!");
        return NULL;
    }

    // open the device file
    if(file == nullptr)
        file = filp_open(device_path, O_RDONLY, 0);

    if (IS_ERR(file)) {
        klog("open", "device file not found!");
        kfree(buffer);
        return NULL;
    }

    // read data from the dev file
    bytes_read = kernel_read(file, buffer, BUFFER_SIZE, &offset);

    // log dev file data
    buffer[bytes_read] = '\0';
    klog(device_path, buffer);

    // clean up and close the file
    filp_close(file, NULL);

    return buffer;
}

static int __init my_module_init(void) {
    klog("init", "Module loaded");
    char* device_data = read_from_device(DEVICE_PATH);
    printk(KERN_INFO "Data read from %s: %s\n", DEVICE_PATH, device_data ? device_data : "NULL");
    kfree(device_data);
    msleep(500);  // Sleep for 500 milliseconds (0.5 second)

    return 0;
}

static void __exit my_module_exit(void) {
    klog("exit", "Module unloaded");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noam Afergan");
MODULE_DESCRIPTION("Control stick data reader with kernel thread");
