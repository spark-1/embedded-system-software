#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("DUAL BSD/GPL");

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "MYDEBUG: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

int my_id = 0;
int get_my_id(void)
{
	return my_id;
}
void set_my_id(int id)
{
	my_id = id;
}
EXPORT_SYMBOL(get_my_id);
EXPORT_SYMBOL(set_my_id);

static int __init ch1_mod1_init(void)
{
	printk(KERN_NOTICE "Hello ch1_Mod1\n");
	DEBUG_MSG("Hello debug world!\n");
	return 0;
}

static void __exit ch1_mod1_exit(void)
{
	printk(KERN_NOTICE "Goodbye ch1_Mod1\n");
}

module_init(ch1_mod1_init);
module_exit(ch1_mod1_exit);
