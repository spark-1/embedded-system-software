#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define IOCTL_START_NUM 0x70
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'x'
#define CORRECT _IO(SIMPLE_IOCTL_NUM, IOCTL_NUM1)
#define INCORRECT _IO(SIMPLE_IOCTL_NUM, IOCTL_NUM2)

#define DEV_NAME "speaker_dev"
#define SPEAKER 12

MODULE_LICENSE("GPL");

static void play(int note) {
	int i = 0;
	for (i = 0; i < 100; i++) {
		gpio_set_value(SPEAKER, 1);
		udelay(note);
		gpio_set_value(SPEAKER, 0);
		udelay(note);
	}
}

static long speaker_correct(void) {
	int i;
	int notes[] = {1516, 1136, 1803, 1516, 1136};

	for (i = 0; i < 5; i++) {
		play(notes[i]);
	}
	gpio_set_value(SPEAKER, 0);

	return 0;
}

static long speaker_incorrect(void) {
	int i;
	int notes[] = {1136};

	for (i = 0; i < 2; i++) {
		play(notes[0]);
	}
	gpio_set_value(SPEAKER, 0);

	return 0;
}

static long speaker_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	switch(cmd) {
		case CORRECT:
			speaker_correct();
			break;
		case INCORRECT:
			speaker_incorrect();
			break;
	}
	return 0;
}

struct file_operations speaker_fops = {
	.unlocked_ioctl = speaker_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init speaker_init(void) {
	int ret;

	printk("speaker: init module\n");

	gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "speaker");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices에서 확인후 메이저번호 동적획득후 써준다
	cd_cdev = cdev_alloc(); // cdev구조체 메모리 할당
	cdev_init(cd_cdev, &speaker_fops); // cdev구조체에 나의 fops 등록
	ret = cdev_add(cd_cdev, dev_num, 1); //디바이스 드라이버 삽입 (cdev리스트에 삽입)
	if (ret < 0) {
		printk("fail to add button character device\n");
		return -1;
	}

	return 0;
}

static void __exit speaker_exit(void) {

	cdev_del(cd_cdev); // 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거

	gpio_set_value(SPEAKER, 0);
	gpio_free(SPEAKER);

	printk("speaker: exit module\n");
}

module_init(speaker_init);
module_exit(speaker_exit);
