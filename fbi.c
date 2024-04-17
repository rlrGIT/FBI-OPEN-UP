/*
 * Boilerplate and makefile from: Daniel Graham
 * Ethical Hacking - A Hands-On Introduction to Breaking In
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>

static int startup(void) {
	printk(KERN_NOTICE "FBI, OPEN UP!");
	return 0;
}

static void shutdown(void) {
	printk(KERN_NOTICE "Closing...");
}

module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");
