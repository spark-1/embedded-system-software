#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // 파일구조체를 위해 inode, file
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "ch3.h"

#define DEV_NAME "ch3_dev"
#define BUF_SIZE 128

MODULE_LICENSE("GPL");

struct msg_list {
        struct list_head list;
        struct msg_st *msg;
};

static struct msg_list msg_list_head;
struct msg_list *tmp = 0;
struct msg_st *kern_buf;
static struct proc_dir_entry *parent_proc = NULL;
spinlock_t my_lock;

static long ch3_read(struct msg_st *user_buf){
	int ret;

    spin_lock(&my_lock);
	if(list_empty(&(msg_list_head.list))) {
		spin_unlock(&my_lock);
		return -1;
	}	
	tmp = list_entry(msg_list_head.list.next, struct msg_list, list);
	kern_buf = tmp->msg;
	printk("ch3: delete from list [%s]\n", tmp->msg->str);
    ret = copy_to_user(user_buf, kern_buf, sizeof(struct msg_st));
	list_del(&(tmp->list));
	
	kfree(tmp);
	vfree(kern_buf);
    spin_unlock(&my_lock);

    return ret;
}

static long ch3_write(struct msg_st *user_buf){
	int ret;

    spin_lock(&my_lock);
    if (user_buf->len >= 128) {
	spin_unlock(&my_lock);
	return -1;
    }
	    
	kern_buf = (struct msg_st *)vmalloc(sizeof(struct msg_st));
	memset(kern_buf, '\0', sizeof(struct msg_st));

    ret = copy_from_user(kern_buf, user_buf, sizeof(struct msg_st));
	tmp = (struct msg_list *)kmalloc(sizeof(struct msg_list), GFP_KERNEL);
    tmp->msg = kern_buf;
    printk("ch3: enter to list [%s]\n", tmp->msg->str);
    list_add_tail(&(tmp->list), &(msg_list_head.list));
    spin_unlock(&my_lock);

    return ret;
}

static long ch3_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	struct msg_st *user_buf;
    int ret = 0;

    user_buf = (struct msg_st *)arg;

    switch(cmd) {
    	case CH3_IOCTL_READ:
        	ret = ch3_read(user_buf);
            printk("ch3: ioctl read return %d\n", ret);
            break;
        case CH3_IOCTL_WRITE:
        	ret = ch3_write(user_buf);
            printk("ch3: ioctl write return %d\n", ret);
            break;
    }
	return ret;
}

static int proc_ch3_show(struct seq_file *seq, void *v) {

	int i = 0;
	list_for_each_entry(tmp, &msg_list_head.list, list){
    	seq_printf(seq, "[%d]: %s\n", i, tmp->msg->str);
        i++;
    }
	return 0;
}

static int ch3_open(struct inode *inode, struct file *file) {

	printk("ch3: open\n");
	return single_open(file, proc_ch3_show, NULL);
}

static int ch3_release(struct inode *inode, struct file *file) {
    printk("ch3: release\n");
    return 0;
}

struct file_operations ch3_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게된다.
	.open = ch3_open,
	.read = seq_read,
    .release = ch3_release,
    .unlocked_ioctl = ch3_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch3_init(void){
	int ret;
	printk("ch3: init module\n");
	
	INIT_LIST_HEAD(&(msg_list_head.list));

    parent_proc = proc_create("ch3_msg", 0444, parent_proc, &ch3_fops); // proc에 파일 생성

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 >동적 할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &ch3_fops); // cdev구조체 초기화
    ret = cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입
	if (ret < 0) {
		printk("fail to add character device\n");
		return -1;
	}

    return 0;
}

static void __exit ch3_exit(void) { 
	struct list_head *pos = 0;
    struct list_head *q = 0;
	int i = 0;

	printk("ch3: exit module\n");

    cdev_del(cd_cdev); // /dev에 디바이스 드라이버 제거
    unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
    proc_remove(parent_proc); // /proc에 만든 파일 제거

	list_for_each_safe(pos, q, &msg_list_head.list) {
    	tmp = list_entry(pos, struct msg_list, list);
        printk("ch3_linked_list: free pos[%d], str[%s]\n", i, tmp->msg->str);
        kfree(tmp);
        i++;
    }
}

module_init(ch3_init);
module_exit(ch3_exit);
                                              
