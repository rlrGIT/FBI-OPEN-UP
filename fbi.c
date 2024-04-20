/*
 * Boilerplate and makefile from: Daniel Graham
 * Ethical Hacking - A Hands-On Introduction to Breaking In
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

static struct kprobe kp  = {
	.symbol_name = "kallsyms_lookup_name"
};

typedef unsigned long (*syscall_addr_t)(const char *name);

void kprobe_addr(void) {
	syscall_addr_t call_ptr;
	register_kprobe(&kp);

	call_ptr = (syscall_addr_t) kp.addr; // not sure what this is doing

	/* instead of using the directly exposed "kallsyms_lookup_name(const char*)"
	 * Can we create a kprobe to look for it and then call it indirectly?
	 *
	 * Alternatively, if we know the relative offset of a syscall in the table,
	 * is there anything that stops us from kprobing a syscall, finding its 
	 * address in memory (where it was called from within the calltable), and 
	 * then subtracting/adding offset to find the pointer to the head of the 
	 * array?
	 *
	 * A lot more reading will need to be done before code is written.
	 */
	printk("Address of lookup function? at %d", *kp.addr);
	unregister_kprobe(&kp);
}

static int startup(void) {
	printk(KERN_NOTICE "FBI, OPEN UP!");
	kprobe_addr();
	return 0;
}

static void shutdown(void) {
	printk(KERN_NOTICE "Closing...");
}

/* These are static */
module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");
