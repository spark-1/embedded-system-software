#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define SWITCH 12

static int irq_num;

static irqreturn_t switch_irq_isr(int irq, void *dev_id) {
	printk("switch_irq: detect\n");
	return IRQ_HANDLED;
}

static int __init switch_irq_init(void) {
	int ret = 0;
	printk("switch_irq: init module\n");
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

	irq_num = gpio_to_irq(SWITCH);
	ret = request_irq(irq_num, switch_irq_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if (ret) {
		printk("switch_irq: unable to reset request IRQ: %d\n", irq_num);
		free_irq(irq_num, NULL);
	}
	else {
		printk("switch_irq: enable to set request IRQ: %d\n", irq_num);
	}
	return 0;
}

static void __exit switch_irq_exit(void) {
	printk("switch_irq: exit module\n");

	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	gpio_free(SWITCH);
}

module_init(switch_irq_init);
module_exit(switch_irq_exit);
