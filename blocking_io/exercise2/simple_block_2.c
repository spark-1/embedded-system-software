#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // 파일구조체를 위해 inode, file
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include "simple_block_2.h"

#define DEV_NAME "simple_block_2_dev"

MODULE_LICENSE("GPL");

spinlock_t my_lock;
static long my_data;
static dev_t dev_num;
static struct cdev *cd_cdev;

wait_queue_head_t my_wq;

static long simple_block_2_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	int ret = 0;

	switch(cmd) {
		case WQ:
			printk("simple_block_2: Process %i (%s) sleep\n", current->pid, current->comm);
			ret = wait_event_interruptible(my_wq, my_data > 0);
			if (ret < 0 ) { // 컨디션이 아닌 시그널에 의해 깨어난 경우
				return ret;
			}
			break;
		case WQ_EX:
			printk("simple_block_2: Process %i (%s) sleep\n", current->pid, current->comm);
			ret = wait_event_interruptible_exclusive(my_wq, my_data > 0);
			if (ret < 0) { // 컨디션이 아닌 시그널에 의해서 깨어난 경우
				return ret;
			}
			break;
		case WQ_WAKE_UP:
			spin_lock(&my_lock);
			my_data = my_data + (long)arg;
			printk("simple_block_2: wake up, update my data\n");
			spin_unlock(&my_lock);
			wake_up_interruptible(&my_wq);
			return my_data;
		case WQ_WAKE_UP_ALL:
			spin_lock(&my_lock);
			my_data = my_data + (long)arg;
			printk("simple_block_2: wake up all\n");
			spin_unlock(&my_lock);
			wake_up_interruptible_all(&my_wq);
			return my_data;
	}

	printk("simple_block_2: process %i (%s) wake up to reduce my data %ld\n", current->pid, current->comm, my_data);
	spin_lock(&my_lock);
	my_data--;
	spin_unlock(&my_lock);

	return my_data;
}
static int simple_block_2_open(struct inode *inode, struct file *file) {

	printk("simple_block_2: open\n");
	return 0;
}

static int simple_block_2_release(struct inode *inode, struct file *file) {
	printk("simple_block_2: release\n");
	return 0;
}

struct file_operations simple_block_2_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게된다.
	.open = simple_block_2_open,
	.release = simple_block_2_release,
	.unlocked_ioctl = simple_block_2_ioctl,
};

static int __init simple_block_2_init(void){
	int ret;
	printk("simple_block_2: init module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 >동적할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &simple_block_2_fops); // cdev구조체 초기화
	ret = cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입

	if (ret < 0) {
		printk("fail to add character device\n");
		return -1;
	}

	spin_lock_init(&my_lock);
	init_waitqueue_head(&my_wq);

	return 0;
}
static void __exit simple_block_2_exit(void) {

	printk("simple_block_2: exit module\n");

	cdev_del(cd_cdev); // /dev에 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
}

module_init(simple_block_2_init);
module_exit(simple_block_2_exit);

