/*
 * Boilerplate and makefile from: Daniel Graham
 * Ethical Hacking - A Hands-On Introduction to Breaking In
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>

static unsigned long *get_table_addr(void) {
	/*
	 * Requires:
	 *
	 * - Custom kernel with exposed method `kallsyms_lookup_name()`
	 *   https://github.com/rlrGIT/linux/tree/v6.1-exposed-symbols
	 *
	 * - Kernel compiled with KALLSYMS=y an KALLSYMS_ALL=y in 
	 *   .config file. This allows the kernel to track and 
	 *   store symbols like 'sys_call_table'.
	 */
	return (unsigned long *) kallsyms_lookup_name("sys_call_table");
}

typedef unsigned long *kspace_addr_t;

static int startup(void) {
	kspace_addr_t sys_call_addr;
	sys_call_addr = get_table_addr();

	printk(KERN_NOTICE "FBI, OPEN UP!");
	if (sys_call_addr) {
		printk("Dumped address of call table at: %lu\n", *sys_call_addr);
	}
	printk("kallsyms_lookup_name returned null pointer.");
	return 0;
}

static void shutdown(void) {
	printk(KERN_NOTICE "Closing...\n");
}

/* These are static */
module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");
