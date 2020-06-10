#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/rculist.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_rcu_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_WRITE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

static dev_t dev_num;
static struct cdev *cd_cdev;
spinlock_t my_lock;

unsigned long __rcu *my_data;

static long simple_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	unsigned long flags;
	unsigned long *new, *old;

	switch(cmd) {
		case IOCTL_READ:
			rcu_read_lock();
			old = rcu_dereference(my_data);
			printk("simple_rcu: read my_data = %ld \n", *old);
			mdelay(500);
			printk("simple_rcu: after delay, read my_data = %ld \n", *old);
			rcu_read_unlock();
			break;
		case IOCTL_WRITE:
			spin_lock_irqsave(&my_lock, flags);
			printk("simple_rcu: write new data = %ld \n", arg);
			new = (unsigned long *)kmalloc(sizeof(unsigned long), GFP_KERNEL);
			*new = arg;
			old = rcu_dereference(my_data);
			rcu_assign_pointer(my_data, new);
			synchronize_rcu();
			kfree(old);
			spin_unlock_irqrestore(&my_lock, flags);
			break;
		default:
			return -1;
	}
	return 0;
}

static int simple_rcu_open(struct inode *inode, struct file *file) {
	return 0;
}

static int simple_rcu_release(struct inode *inode, struct file *file) {
	return 0;
}

struct file_operations simple_rcu_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게 된다
	.open = simple_rcu_open,
	.release = simple_rcu_release,
	.unlocked_ioctl = simple_ioctl,
};

static int __init simple_rcu_init(void) {
	unsigned long *data;

	printk("simple_rcu: init module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 >동적 할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &simple_rcu_fops); // cdev구조체 초기화
	cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입

	spin_lock_init(&my_lock);

	data = (unsigned long *)kmalloc(sizeof(unsigned long), GFP_KERNEL);
	*data = 0;
	rcu_assign_pointer(my_data, data);

	return 0;
}

static void __exit simple_rcu_exit(void) {

	unsigned long flags;
	unsigned long *data;

	printk("simple_rwlock: exit module\n");
	
	spin_lock_irqsave(&my_lock, flags);
	data = rcu_dereference(my_data);
	rcu_assign_pointer(my_data, NULL);
	kfree(data);
	spin_unlock_irqrestore(&my_lock, flags);

	cdev_del(cd_cdev); // /dev 에 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
}

module_init(simple_rcu_init);
module_exit(simple_rcu_exit);
