#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define SENSOR1 17
#define LED1 5

static int irq_num;

struct my_timer_info {
	struct timer_list timer;
	long delay_jiffies;
	int data;
};

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t) {
	struct my_timer_info *info = from_timer(info, t, timer);
	printk("ch8_mod: data %d led off\n", info->data);
	gpio_set_value(LED1, 0);
}

static irqreturn_t ch8_mod_isr(int irq, void *dev_id) {
	unsigned long flags;
	local_irq_save(flags);
	my_timer.data++;
	gpio_set_value(LED1, 0);
	gpio_set_value(LED1, 1);
	mod_timer(&my_timer.timer, jiffies + my_timer.delay_jiffies);
	local_irq_restore(flags);
	printk("ch8_mod: detect data %d\n", my_timer.data);
	return IRQ_HANDLED;
}

static int __init ch8_mod_init(void) {
	int ret = 0;

	printk("ch8_mod: init module\n");

	my_timer.delay_jiffies = msecs_to_jiffies(2000);
	my_timer.data = 0;
	timer_setup(&my_timer.timer, my_timer_func, 0);
	my_timer.timer.expires = jiffies + my_timer.delay_jiffies;

	gpio_request_one(SENSOR1, GPIOF_IN, "SENSOR1");
	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	irq_num = gpio_to_irq(SENSOR1);

	ret = request_irq(irq_num, ch8_mod_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if (ret) {
		printk("ch8_mod: unable to reset request IRQ: %d\n", irq_num);
		free_irq(irq_num, NULL);
	}
	else {
		printk("ch8_mod: enable to set request IRQ: %d\n", irq_num);
	}
	return 0;
}

static void __exit ch8_mod_exit(void) {
	printk("ch8_mod: exit module\n");
	
	del_timer(&my_timer.timer);
	free_irq(irq_num, NULL);
	gpio_free(SENSOR1);
	gpio_free(LED1);
}

module_init(ch8_mod_init);
module_exit(ch8_mod_exit);
