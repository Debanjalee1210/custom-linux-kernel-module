// rgb_led.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "led_ioctl.h"



#define DEV_NAME "rgbled"

static int major;
static char led_state[16] = "OFF";

static struct led_desc config;

static long dev_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case CONFIG:
			if(copy_from_user(&config,(void __user*)arg,sizeof(struct led_desc)))
				return -EFAULT;
			pr_info("[rgbled] IOCTL CONFIG received: PWM=%d, R=%d, G=%d, B=%d\n",config.PWM, config.R, config.G, config.B);
			return 0;
		default:
			return -EINVAL;
	}
}

static ssize_t dev_write(struct file *f, const char *buf, size_t len, loff_t *off) {
    if (len > sizeof(led_state)-1)
        len = sizeof(led_state)-1;
    if (copy_from_user(led_state, buf, len))
        return -EFAULT;
    led_state[len] = '\0';
    pr_info("LED State changed to: %s\n", led_state);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,  
};

static int __init rgbled_init(void) {
    major = register_chrdev(0, DEV_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register device\n");
        return major;
    }
    pr_info("rgbled device registered with major %d\n", major);
    return 0;
}

static void __exit rgbled_exit(void) {
    unregister_chrdev(major, DEV_NAME);
    pr_info("rgbled device unregistered\n");
}

module_init(rgbled_init);
module_exit(rgbled_exit);

MODULE_LICENSE("GPL");

