#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode, file
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <asm/delay.h>
#include "simple_spin.h"

#define DEV_NAME "simple_spin_dev"


MODULE_LICENSE("GPL");
struct str_st *kern_buf;
spinlock_t my_lock;

void delay(int sec) {
	int i, j;
	for(j = 0; j < sec; j++) {
		for (i = 0; i < 1000; i++) {
			udelay(1000);
		}
	}
}

static long simple_spin_read(struct str_st *buf){
		int ret;

		delay(5);
		spin_lock(&my_lock);
		ret = copy_to_user(buf, kern_buf, sizeof(struct str_st));
		memset(kern_buf, '\0', sizeof(struct str_st));
		spin_unlock(&my_lock);
        
		return ret;
}

static long simple_spin_write(struct str_st *buf){
		int ret;

		spin_lock(&my_lock);
		ret = copy_from_user(kern_buf, buf, sizeof(struct str_st));
		spin_unlock(&my_lock);
        
		return ret;
}

static long simple_spin_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	struct str_st *user_buf;
	int ret;

	user_buf = (struct str_st *)arg;

        switch(cmd) {
        	case SIMPLE_SPIN_READ:
			ret = simple_spin_read(user_buf);
                	printk("simple_spin: ioctl read return %d\n", ret);
			break;
        	case SIMPLE_SPIN_WRITE:
			ret = simple_spin_write(user_buf);
                	printk("simple_spin: ioctl write return %d\n", ret);
			break;
		default:
			break;
        }
        return 0;
}

static int simple_spin_open(struct inode *inode, struct file *file) {
        printk("simple_spin: open\n");
        return 0;
}

static int simple_spin_release(struct inode *inode, struct file *file) {
        printk("simple_spin: release\n");
        return 0;
}

struct file_operations simple_spin_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게된다.
        .open = simple_spin_open,
        .release = simple_spin_release,
        .unlocked_ioctl = simple_spin_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_spin_init(void) { // insmod시 호출
	int ret;
        printk("simple_spin: init module\n");
	
	spin_lock_init(&my_lock);
        alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 동적 할당후 등록
        cd_cdev = cdev_alloc(); // cdev구조체 초기화
        cdev_init(cd_cdev, &simple_spin_fops); // cdev구조체 초기화
        ret = cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입
	if (ret < 0) {
		printk("fail to add character device\n");
		return -1;
	}
	kern_buf = (struct str_st *)vmalloc(sizeof(struct str_st));
	memset(kern_buf, '\0', sizeof(struct str_st));

        return 0;
}

static void __exit simple_spin_exit(void) { // rmmod시 호출
        printk("simple_spin: exit module\n");

        cdev_del(cd_cdev); // /dev에 디바이스 드라이버 제거
        unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
	vfree(kern_buf);
}

module_init(simple_spin_init);
module_exit(simple_spin_exit);

