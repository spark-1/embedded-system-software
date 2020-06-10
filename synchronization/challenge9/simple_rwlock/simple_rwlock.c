#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/rwlock.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_rwlock_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_WRITE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

static dev_t dev_num;
static struct cdev *cd_cdev;
rwlock_t my_lock;

unsigned long *my_data;

static long simple_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	unsigned long flags;
	unsigned long *new, *old;

	switch(cmd) {
		case IOCTL_READ:
			read_lock_irqsave(&my_lock, flags);
			old = my_data;
			printk("simple_rwlock: read my_data = %ld \n", *old);
			mdelay(500);
			printk("simple_rwlock: after delay, read my_data = %ld \n", *old);
			read_unlock_irqrestore(&my_lock, flags);
			break;
		case IOCTL_WRITE:
			write_lock_irqsave(&my_lock, flags);
			printk("simple_rwlock: write new data = %ld \n", arg);
			new = (unsigned long *)kmalloc(sizeof(unsigned long), GFP_KERNEL);
			*new = arg;
			old = my_data;
			my_data = new;
			mdelay(200);
			kfree(old);
			write_unlock_irqrestore(&my_lock, flags);
			break;
		default:
			return -1;
	}
	return 0;
}

static int simple_rwlock_open(struct inode *inode, struct file *file) {
	return 0;
}

static int simple_rwlock_release(struct inode *inode, struct file *file) {
	return 0;
}

struct file_operations simple_rwlock_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게 된다
	.open = simple_rwlock_open,
	.release = simple_rwlock_release,
	.unlocked_ioctl = simple_ioctl,
};

static int __init simple_rwlock_init(void) {

	printk("simple_rwlock: init module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 동적 할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &simple_rwlock_fops); // cdev구조체 초기화
	cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입

	rwlock_init(&my_lock);

	my_data = (unsigned long *)kmalloc(sizeof(unsigned long), GFP_KERNEL);
	*my_data = 0;

	return 0;
}

static void __exit simple_rwlock_exit(void) {
	printk("simple_rwlock: exit module\n");

	kfree(my_data);
	cdev_del(cd_cdev); // /dev 에 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
}

module_init(simple_rwlock_init);
module_exit(simple_rwlock_exit);
