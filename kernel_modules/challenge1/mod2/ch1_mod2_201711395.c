#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("DUAL BSD/GPL");

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "MYDEBUG: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

extern int get_my_id(void);
extern void set_my_id(int id);

static int __init ch1_mod2_init(void)
{
	printk(KERN_NOTICE "Hello ch1_Mod2\n");
	printk(KERN_NOTICE "My ID : %d\n", get_my_id());
	set_my_id(201711395);
	printk(KERN_NOTICE "My ID : %d\n", get_my_id());
	DEBUG_MSG("Hello debug world!\n");
	return 0;
}

static void __exit ch1_mod2_exit(void)
{
	printk(KERN_NOTICE "Goodbye ch1_Mod2\n");
}

module_init(ch1_mod2_init);
module_exit(ch1_mod2_exit);
