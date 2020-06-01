#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define SWITCH 12
#define LED1 5

struct my_timer_info {
	struct timer_list timer;
	long delay_jiffies;
	int data;
};

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t) {
	int ret = 0;
	struct my_timer_info *info = from_timer(info, t, timer);

	ret = gpio_get_value(SWITCH);
	printk("ch7_mod: jiffies %ld, Data %d, switch %d\n", jiffies, info->data, ret);
	
	if(ret) {
		gpio_set_value(LED1, 1);
	}
	else {
		gpio_set_value(LED1, 0);
	}
	
	info->data++;
	mod_timer(&my_timer.timer, jiffies + info->delay_jiffies);
}

static int __init ch7_mod_init(void) {
	printk("ch7_mod: init module\n");

	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");
	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");

	my_timer.delay_jiffies = msecs_to_jiffies(500); // 0.5초
	my_timer.data = 0;
	timer_setup(&my_timer.timer, my_timer_func, 0);
	my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
	add_timer(&my_timer.timer);

	return 0;
}

static void __exit ch7_mod_exit(void) {
	printk("ch7_mod: exit module\n");
	del_timer(&my_timer.timer);
	gpio_set_value(LED1, 0);
	gpio_free(SWITCH);
	gpio_free(LED1);
}

module_init(ch7_mod_init);
module_exit(ch7_mod_exit);
