#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("DUAL BSD/GPL");

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "MYDEBUG: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

int my_id = 202077777;
EXPORT_SYMBOL(my_id);

static int __init mod1_init(void)
{
	printk(KERN_NOTICE "Hello Mod1\n");
	DEBUG_MSG("Hello debug world!\n");
	return 0;
}

static void __exit mod1_exit(void)
{
	printk(KERN_NOTICE "Goodbye Mod1\n");
}

module_init(mod1_init);
module_exit(mod1_exit);
