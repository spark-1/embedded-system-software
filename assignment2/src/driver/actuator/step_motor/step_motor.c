#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define IOCTL_START_NUM 0x90
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'y'
#define OPEN _IO(SIMPLE_IOCTL_NUM, IOCTL_NUM1)
#define CLOSE _IO(SIMPLE_IOCTL_NUM, IOCTL_NUM2)

#define DEV_NAME "step_motor_dev"
#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define DEGREE 128
#define DELAY 3
#define STEPS 8

MODULE_LICENSE("GPL");

int blue[8] = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

void setstep(int p1, int p2, int p3, int p4){	
	gpio_set_value(PIN1, p1);
	gpio_set_value(PIN2, p2);
	gpio_set_value(PIN3, p3);
	gpio_set_value(PIN4, p4);
}

static long step_motor_open(void) {
	int i = 0, j = 0;

	int degree = DEGREE;

	for(i = 0; i < degree; i++){
		for(j = 0; j < STEPS; j++){
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			mdelay(DELAY);
		}
	}
	setstep(0, 0, 0, 0);

	return 0;
}

static long step_motor_close(void) {
	int i = 0, j = 0;

	int degree = DEGREE;

	for(i = 0; i < degree; i++){
		for(j = STEPS - 1; j >= 0; j--){
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			mdelay(DELAY);
		}
	}
	setstep(0, 0, 0, 0);

	return 0;
}

static long step_motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	switch(cmd) {
		case OPEN:
			step_motor_open();
			break;
		case CLOSE:
			step_motor_close();
			break;
	}
	return 0;
}

struct file_operations step_motor_fops = {
	.unlocked_ioctl = step_motor_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init step_motor_init(void) {
	int ret;

	printk("step_motor: init module\n");

	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices에서 확인후 메이저번호 동적획득후 써준다
	cd_cdev = cdev_alloc(); // cdev구조체 메모리 할당
	cdev_init(cd_cdev, &step_motor_fops); // cdev구조체에 나의 fops 등록
	ret = cdev_add(cd_cdev, dev_num, 1); //디바이스 드라이버 삽입 (cdev리스트에 삽입)
	if (ret < 0) {
		printk("fail to add button character device\n");
		return -1;
	}

	return 0;
}

static void __exit step_motor_exit(void) {

	cdev_del(cd_cdev); // 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거

	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);

	printk("step_motor: exit module\n");
}

module_init(step_motor_init);
module_exit(step_motor_exit);
