/*
 * Exposed symbol boilerplate and makefile from: Daniel Graham
 * Ethical Hacking - A Hands-On Introduction to Breaking In
 *
 * https://docs.kernel.org/trace/kprobes.html
 */ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>

#define PROC_FAIL 1;

int lookup_prehandler(struct kprobe *probe, struct pt_regs *regs) {
	printk(KERN_NOTICE "Attempting kprobe address dump...");
	return 0;
}

typedef unsigned long (*pt_kallsyms_lookup_name)(const char *);

unsigned long find_ksymbol(const char *name) {
/*
 * These preprocessor macros are kinda cool,
 * but they don't seem scalable. This works...
 * for now.
 */
#ifdef kallsyms_lookup_name
	printk("Found exposed symbol kallsyms_lookup_name.");
	return kallsyms_lookup_name(name);
#endif
#ifdef CONFIG_KPROBES
	pt_kallsyms_lookup_name lookup;

	struct kprobe probe = {
		.symbol_name = "kallsyms_lookup_name",
		.pre_handler = lookup_prehandler,
	};
	
	if (register_kprobe(&probe) != 0) {
		printk(KERN_ERR "Registering kallsyms probe failed.");
		return -PROC_FAIL;
	}

	lookup = (pt_kallsyms_lookup_name) probe.addr;
	unregister_kprobe(&probe);
	printk(KERN_NOTICE "Got kallsyms_lookup_name address: %lu.",
		(unsigned long) lookup);

	return lookup(name);
#endif
	// TODO: add linear address search from through exposed symbols?
	// Add some functionality if kprobes aren't available
	return -PROC_FAIL;
}

// TODO: should cache the table address somehow when we start using it
// to grab syscalls

static int startup(void) {
	unsigned long sys_call_table_addr;
	sys_call_table_addr = find_ksymbol("sys_call_table");
	if (sys_call_table_addr <= 0) {
		printk(KERN_ERR "Could not find symbol.");
	} else {
		printk(KERN_NOTICE "Dumped call table at: %lu.\n",
			(unsigned long) sys_call_table_addr);
	}
	return 0;
}

static void shutdown(void) {
	printk(KERN_NOTICE "Closing...\n");
}

module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");
