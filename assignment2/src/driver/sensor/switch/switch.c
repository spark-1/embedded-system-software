#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // 파일구조체를 위해 inode, file
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <asm/delay.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define SIMPLE_IOCTL_NUM 'z'
#define SWITCH_QUEUE_GET _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define SWITCH_QUEUE_CLOSE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long)
#define SWITCH_RECV _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long)
#define ULTRASONIC_RECV _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long)

#define DEV_NAME "switch_dev"
#define ULTRA_TRIG 17
#define ULTRA_ECHO 18
#define SWITCH1 20
#define SWITCH2 21
#define SWITCH3 22
#define QUEUE_NUM 10

MODULE_LICENSE("GPL");

struct data_t {
	int value;
	unsigned long timestamp;
};

struct switch_queue {
	struct list_head list;
	struct data_t data;
};

static int irq_num[4];
static struct switch_queue switch_queue_head[QUEUE_NUM]; // 프로세스마다 쓰는 큐
static struct switch_queue ultrasonic_queue_head[QUEUE_NUM]; // 프로세스마다 쓰는 큐
struct switch_queue *tmp; 
struct tasklet_struct switch_tasklet;
struct tasklet_struct ultrasonic_tasklet;
struct data_t switch_data_tmp;
struct data_t ultrasonic_data_tmp;
spinlock_t my_lock;
unsigned long pre; // 버튼 중복 방지용 변수
int queue_use[QUEUE_NUM]; // 생성된 큐 1, 생성되지 않은 큐 0

static int echo_valid_flag = 3;
static ktime_t echo_start;
static ktime_t echo_stop;

static long switch_queue_get(int key) {
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&my_lock, flags);
	if (queue_use[key] || key < 0 || key >= QUEUE_NUM) {
		ret = -1;
	}
	else {
		queue_use[key] = 1;
		ret = 1;
	}
	spin_unlock_irqrestore(&my_lock, flags);

	return ret;
}

static long switch_queue_close(int key) {
	int ret;
	unsigned long flags;
	struct list_head *pos = 0;
	struct list_head *q = 0;

	spin_lock_irqsave(&my_lock, flags);
	if (!queue_use[key] || key < 0 || key >= QUEUE_NUM) {
		ret = -1;
	}
	else {
		list_for_each_safe(pos, q, &switch_queue_head[key].list) {
			tmp = list_entry(pos, struct switch_queue, list);
			list_del(pos);
			kfree(tmp);
		}
		list_for_each_safe(pos, q, &ultrasonic_queue_head[key].list) {
			tmp = list_entry(pos, struct switch_queue, list);
			list_del(pos);
			kfree(tmp);
		}
		queue_use[key] = 0;
		ret = 1;
	}
	spin_unlock_irqrestore(&my_lock, flags);

	return ret;
}

static long switch_recv(int key) {
	int ret;
	unsigned long flags, time;

	spin_lock_irqsave(&my_lock, flags);
	if (list_empty(&switch_queue_head[key].list)) {
		ret = -1;
	}
	else {
		tmp = list_entry(switch_queue_head[key].list.next, struct switch_queue, list);
		ret = tmp->data.value;
		time = tmp->data.timestamp;
		list_del(&tmp->list);
		kfree(tmp);
		printk("switch_recv: value %d, jiffies %lu recv\n", ret, time);
	}
	spin_unlock_irqrestore(&my_lock, flags);
	return ret;
}

static long ultrasonic_recv(int key) {
	int ret;
	unsigned long flags, time;

	spin_lock_irqsave(&my_lock, flags);
	if (list_empty(&ultrasonic_queue_head[key].list)) {
		ret = -1;
	}
	else {
		tmp = list_entry(ultrasonic_queue_head[key].list.next, struct switch_queue, list);
		ret = tmp->data.value;
		time = tmp->data.timestamp;
		list_del(&tmp->list);
		kfree(tmp);
		printk("ultrasonic_recv: value %d, jiffies %lu recv\n", ret, time);
	}
	spin_unlock_irqrestore(&my_lock, flags);
	return ret;
}

void switch_tasklet_func(unsigned long recv_data) {
	unsigned long flags;
	struct data_t *my_data = (struct data_t *)recv_data;
	int i;

	spin_lock_irqsave(&my_lock, flags);
	if (my_data->timestamp > pre + 20) {
		printk("switch value:%d timestamp:%lu\n", my_data->value, my_data->timestamp);
		for (i = 0; i < QUEUE_NUM; i++) {
			if (queue_use[i]) {
				tmp = (struct switch_queue *)kmalloc(sizeof(struct switch_queue), GFP_ATOMIC);
				tmp->data.value = my_data->value;
				tmp->data.timestamp = my_data->timestamp;
				list_add_tail(&tmp->list, &switch_queue_head[i].list);
			}
		}
		pre = my_data->timestamp;

		if(echo_valid_flag == 3) { // ultrasonic 작동
			echo_start = ktime_set(0, 1);
			echo_stop = ktime_set(0, 1);
			echo_valid_flag = 0;

			gpio_set_value(ULTRA_TRIG, 1);
			udelay(10);
			gpio_set_value(ULTRA_TRIG, 0);

			echo_valid_flag = 1;
		}

	}
	spin_unlock_irqrestore(&my_lock, flags);
}

void ultrasonic_tasklet_func(unsigned long recv_data) {
	unsigned long flags;
	struct data_t *my_data = (struct data_t *)recv_data;
	int i;

	spin_lock_irqsave(&my_lock, flags);
	printk("ultrasonic value:%d timestamp:%lu\n", my_data->value, my_data->timestamp);
	for (i = 0; i < QUEUE_NUM; i++) {
		if (queue_use[i]) {
			tmp = (struct switch_queue *)kmalloc(sizeof(struct switch_queue), GFP_ATOMIC);
			tmp->data.value = my_data->value;
			tmp->data.timestamp = my_data->timestamp;
			list_add_tail(&tmp->list, &ultrasonic_queue_head[i].list);
		}
	}
	spin_unlock_irqrestore(&my_lock, flags);
}

static irqreturn_t switch1_irq_isr(int irq, void *dev_id) {
	printk("button1_irq: detect\n");
	switch_data_tmp.value = 1;
	switch_data_tmp.timestamp = jiffies;
	tasklet_schedule(&switch_tasklet);
	return IRQ_HANDLED;
}

static irqreturn_t switch2_irq_isr(int irq, void *dev_id) {
	printk("button2_irq: detect\n");
	switch_data_tmp.value = 2;
	switch_data_tmp.timestamp = jiffies;
	tasklet_schedule(&switch_tasklet);
	return IRQ_HANDLED;
}

static irqreturn_t switch3_irq_isr(int irq, void *dev_id) {
	printk("button3_irq: detect\n");
	switch_data_tmp.value = 3;
	switch_data_tmp.timestamp = jiffies;
	tasklet_schedule(&switch_tasklet);
	return IRQ_HANDLED;
}

static irqreturn_t ultrasonic_isr(int irq, void *dev_id) {

	ktime_t tmp_time;
	s64 time;
	int cm;

	tmp_time = ktime_get();
	if(echo_valid_flag == 1) {
		printk("ultrasonic: echo up\n");
		if(gpio_get_value(ULTRA_ECHO) == 1) {
			echo_start = tmp_time;
			echo_valid_flag = 2;
		}
	} else if(echo_valid_flag == 2) {
		printk("ultrasonic: echo down\n");
		if(gpio_get_value(ULTRA_ECHO) == 0) {
			echo_stop = tmp_time;
			time = ktime_to_us(ktime_sub(echo_stop, echo_start));
			cm = (int)time / 58;
			printk("ultrasonic: detect %d cm\n", cm);
			echo_valid_flag = 3;
			ultrasonic_data_tmp.value = cm;
			ultrasonic_data_tmp.timestamp = jiffies;
			tasklet_schedule(&ultrasonic_tasklet);
		}
	}
	return IRQ_HANDLED;
}

static long switch_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret = 0, key;

	switch(cmd) {
		case SWITCH_QUEUE_GET:
			key = (int)arg;
			ret = switch_queue_get(key);
			break;
		case SWITCH_QUEUE_CLOSE:
			key = (int)arg;
			ret = switch_queue_close(key);
			break;
		case SWITCH_RECV:
			key = (int)arg;
			ret = switch_recv(key);
			break;
		case ULTRASONIC_RECV:
			key = (int)arg;
			ret = ultrasonic_recv(key);
			break;
	}
	return ret;
}

static int switch_open(struct inode *inode, struct file *file) {
	//printk("switch: open\n");
	return 0;
}

static int switch_release(struct inode *inode, struct file *file) {
	//printk("switch: release\n");
	return 0;
}

struct file_operations switch_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값
	.open = switch_open,
	.release = switch_release,
	.unlocked_ioctl = switch_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init switch_init(void) {
	int ret, i;
	printk("switch: init module\n");

	tasklet_init(&switch_tasklet, switch_tasklet_func, (unsigned long)&switch_data_tmp);
	tasklet_init(&ultrasonic_tasklet, ultrasonic_tasklet_func, (unsigned long)&ultrasonic_data_tmp);

	gpio_request_one(SWITCH1, GPIOF_IN, "SWITCH1");
	gpio_request_one(SWITCH2, GPIOF_IN, "SWITCH2");
	gpio_request_one(SWITCH3, GPIOF_IN, "SWITCH3");
	gpio_request_one(ULTRA_TRIG, GPIOF_OUT_INIT_LOW, "ULTRA_TRIG");
	gpio_request_one(ULTRA_ECHO, GPIOF_IN, "ULTRA_ECHO");

	irq_num[0] = gpio_to_irq(SWITCH1);
	irq_num[1] = gpio_to_irq(SWITCH2);
	irq_num[2] = gpio_to_irq(SWITCH3);
	irq_num[3] = gpio_to_irq(ULTRA_ECHO);

	ret = request_irq(irq_num[0], switch1_irq_isr, IRQF_TRIGGER_RISING, "switch1_irq", NULL);
	if(ret) {
		printk("switch1_irq unable to request IRQ");
		free_irq(irq_num[0], NULL);
		return -1;
	}
	ret = request_irq(irq_num[1], switch2_irq_isr, IRQF_TRIGGER_RISING, "switch2_irq", NULL);
	if(ret) {
		printk("switch2_irq unable to request IRQ");
		free_irq(irq_num[1], NULL);
		return -1;
	}
	ret = request_irq(irq_num[2], switch3_irq_isr, IRQF_TRIGGER_RISING, "switch3_irq", NULL);
	if(ret) {
		printk("switch3_irq unable to request IRQ");
		free_irq(irq_num[2], NULL);
		return -1;
	}
	ret = request_irq(irq_num[3], ultrasonic_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "ULTRA_ECHO", NULL);
	if(ret) {
		printk("ultrasonic_irq unable to request IRQ");
		free_irq(irq_num[3], NULL);
		return -1;
	}

	for(i = 0; i < QUEUE_NUM; i++) {
		INIT_LIST_HEAD(&switch_queue_head[i].list);
	}
	for(i = 0; i < QUEUE_NUM; i++) {
		INIT_LIST_HEAD(&ultrasonic_queue_head[i].list);
	}

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices에서 확인후 메이저번호 동적획득후 써준다
	cd_cdev = cdev_alloc(); // cdev구조체 메모리 할당
	cdev_init(cd_cdev, &switch_fops); // cdev구조체에 나의 fops 등록
	ret = cdev_add(cd_cdev, dev_num, 1); //디바이스 드라이버 삽입 (cdev리스트에 삽입)
	if (ret < 0) {
		printk("fail to add switch character device\n");
		return -1;
	}

	return 0;
}

static void __exit switch_exit(void) {
	struct list_head *pos = 0;
	struct list_head *q = 0;
	int i = 0;
	unsigned long flags;

	tasklet_kill(&switch_tasklet);
	tasklet_kill(&ultrasonic_tasklet);

	cdev_del(cd_cdev); // 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거

	free_irq(irq_num[0], NULL);
	free_irq(irq_num[1], NULL);
	free_irq(irq_num[2], NULL);
	free_irq(irq_num[3], NULL);
	gpio_free(SWITCH1);
	gpio_free(SWITCH2);
	gpio_free(SWITCH3);
	gpio_free(ULTRA_TRIG);
	gpio_free(ULTRA_ECHO);

	spin_lock_irqsave(&my_lock, flags);
	for (i = 0; i < QUEUE_NUM; i++) {
		list_for_each_safe(pos, q, &switch_queue_head[i].list) {
			tmp = list_entry(pos, struct switch_queue, list);
			list_del(pos);
			kfree(tmp);
		}
	}
	for (i = 0; i < QUEUE_NUM; i++) {
		list_for_each_safe(pos, q, &ultrasonic_queue_head[i].list) {
			tmp = list_entry(pos, struct switch_queue, list);
			list_del(pos);
			kfree(tmp);
		}
	}
	spin_unlock_irqrestore(&my_lock, flags);

	printk("switch: exit module\n");
}

module_init(switch_init);
module_exit(switch_exit);
