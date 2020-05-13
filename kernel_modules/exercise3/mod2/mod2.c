#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("DUAL BSD/GPL");

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "MYDEBUG: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

extern int my_id;

static int __init mod2_init(void)
{
	printk(KERN_NOTICE "Hello Mod2\n");
	printk(KERN_NOTICE "My ID : %d\n", my_id);
	DEBUG_MSG("Hello debug world!\n");
	return 0;
}

static void __exit mod2_exit(void)
{
	printk(KERN_NOTICE "Goodbye Mod2\n");
}

module_init(mod2_init);
module_exit(mod2_exit);
