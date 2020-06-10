#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/rculist.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_rculist_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_UPDATE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

struct test_list {
	struct list_head list;
	unsigned long id;
	unsigned long data;
};

static dev_t dev_num;
static struct cdev *cd_cdev;
struct test_list __rcu mylist;
spinlock_t my_lock;

static long simple_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	unsigned long flags;
	struct test_list *pos;
	struct test_list *new;
	struct test_list *old;

	switch(cmd) {
		case IOCTL_READ:
			rcu_read_lock();
			printk("simple_rculist: read\n");
			list_for_each_entry_rcu(pos, &mylist.list, list) {
				if(pos->id == arg) {
					printk("simple_rculist: list [id = %ld] = %ld\n", arg, pos->data);
					rcu_read_unlock();
					return 0;
				}
			}
			printk("simple_rculist: read not found id = %ld\n", arg);
			rcu_read_unlock();
			break;
		case IOCTL_UPDATE:
			spin_lock_irqsave(&my_lock, flags);
			printk("simple_rculist: update\n");
			list_for_each_entry_rcu(pos, &mylist.list, list) {
				if(pos->id == arg) {
					old = pos;
					new = (struct test_list *)kmalloc(sizeof(struct test_list), GFP_KERNEL);
					memcpy(new, old, sizeof(struct test_list));
					new->data++;
					list_replace_rcu(&pos->list, &new->list);
					synchronize_rcu();
					kfree(old);
					printk("simple_rculist: list [id = %ld] = %ld\n", arg, new->data);
					spin_unlock_irqrestore(&my_lock, flags);
					return 0;
				}
			}
			printk("simple_rculist: update not found id = %ld\n", arg);
			spin_unlock_irqrestore(&my_lock, flags);
			break;
		default:
			return -1;
	}
	return 0;
}

static int simple_rculist_open(struct inode *inode, struct file *file) {
	return 0;
}

static int simple_rculist_release(struct inode *inode, struct file *file) {
	return 0;
}

struct file_operations simple_rculist_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게 된다
	.open = simple_rculist_open,
	.release = simple_rculist_release,
	.unlocked_ioctl = simple_ioctl,
};

static int __init simple_rculist_init(void) {

	int i;
	struct test_list *pos;

	printk("simple_rculist: init module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 >동적 할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &simple_rculist_fops); // cdev구조체 초기화
	cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입

	spin_lock_init(&my_lock);
	INIT_LIST_HEAD(&mylist.list);

	for (i = 0; i < 5; i++) {
		pos = (struct test_list *)kmalloc(sizeof(struct test_list), GFP_KERNEL);
		pos->id = i + 1;
		pos->data = 100;
		list_add_tail_rcu(&(pos->list), &(mylist.list));
	}

	return 0;
}

static void __exit simple_rculist_exit(void) {
	struct test_list *pos;
	struct test_list *tmp;
	unsigned long flags;

	spin_lock_irqsave(&my_lock, flags);

	list_for_each_entry_safe(tmp, pos, &mylist.list, list) {
		list_del_rcu(&(tmp->list));
		synchronize_rcu();
		kfree(tmp);
	}

	spin_unlock_irqrestore(&my_lock, flags);

	cdev_del(cd_cdev); // /dev 에 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거

	printk("simple_rculist: exit module\n");
}

module_init(simple_rculist_init);
module_exit(simple_rculist_exit);
