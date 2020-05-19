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
#include "ch4.h"

#define DEV_NAME "ch4_dev"
#define BUF_SIZE 128

MODULE_LICENSE("GPL");

struct msg_list {
        struct list_head list;
        struct msg_st *msg;
};

static struct msg_list msg_list_head; // 메세지 리스트 헤드
struct msg_list *tmp = 0;
struct msg_st *kern_buf; // 커널 버퍼
static struct proc_dir_entry *parent_proc = NULL; // /proc/ 디렉토리
static int msg_count = 0;
spinlock_t my_lock;

wait_queue_head_t my_wq;

static long ch4_read(struct msg_st *user_buf){
	int ret;

    spin_lock(&my_lock);
	if(list_empty(&(msg_list_head.list))) { // 메세지 리스트에 메세지가 없는 경우
		spin_unlock(&my_lock);
		wait_event_interruptible(my_wq, msg_count > 0); // 메세지가 생길때까지 잠든다
		spin_lock(&my_lock);
	}	
	tmp = list_entry(msg_list_head.list.next, struct msg_list, list);
	kern_buf = tmp->msg;
	printk("ch4: proceess %i delete from list [%s]\n", current->pid, tmp->msg->str);
    ret = copy_to_user(user_buf, kern_buf, sizeof(struct msg_st)); // 성공시 0 리턴
	list_del(&(tmp->list)); // 뽑아낸 첫번째 리스트 노드 삭제
	
	kfree(tmp); // 삭제 노드 프리
	vfree(kern_buf); // 노드 안 메세지도 프리
	msg_count--;
    spin_unlock(&my_lock);

    return ret; // 0 리턴시 성공적
}

static long ch4_write(struct msg_st *user_buf){
	int ret;

    spin_lock(&my_lock);
    if (user_buf->len >= BUF_SIZE) { // 버퍼사이즈가 넘는 문자열은 받지 않는다. 
		spin_unlock(&my_lock);
		return -1;
    }
	    
	kern_buf = (struct msg_st *)vmalloc(sizeof(struct msg_st));
	memset(kern_buf, '\0', sizeof(struct msg_st));

    ret = copy_from_user(kern_buf, user_buf, sizeof(struct msg_st));
	tmp = (struct msg_list *)kmalloc(sizeof(struct msg_list), GFP_KERNEL);
    tmp->msg = kern_buf;
    printk("ch4: process %i enter to list [%s]\n", current->pid, tmp->msg->str);
    list_add_tail(&(tmp->list), &(msg_list_head.list));
	msg_count++;
	wake_up_interruptible(&my_wq);
    spin_unlock(&my_lock);

    return ret; // 성공시 0 리턴
}

static long ch4_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	struct msg_st *user_buf;
    int ret = 0;

    user_buf = (struct msg_st *)arg;

    switch(cmd) {
    	case CH4_IOCTL_READ:
        	ret = ch4_read(user_buf);
            printk("ch4: ioctl read return %d\n", ret);
            break;
        case CH4_IOCTL_WRITE:
        	ret = ch4_write(user_buf);
            printk("ch4: ioctl write return %d\n", ret);
            break;
    }
	return ret;
}

static int proc_ch4_show(struct seq_file *seq, void *v) {

	int i = 0;
	list_for_each_entry(tmp, &msg_list_head.list, list){
    	seq_printf(seq, "[%d]: %s\n", i, tmp->msg->str);
        i++;
    }
	return 0;
}

static int ch4_open(struct inode *inode, struct file *file) {

	printk("ch4: open\n");
	return single_open(file, proc_ch4_show, NULL);
}

static int ch4_release(struct inode *inode, struct file *file) {
    printk("ch4: release\n");
    return 0;
}

struct file_operations ch4_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게된다.
	.open = ch4_open,
	.read = seq_read,
    .release = ch4_release,
    .unlocked_ioctl = ch4_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch4_init(void){
	int ret;
	printk("ch4: init module\n");
	
	INIT_LIST_HEAD(&(msg_list_head.list)); // 메세지 리스트 초기화
	spin_lock_init(&my_lock); // 스핀락 변수 초기화
	init_waitqueue_head(&my_wq); // 큐 초기화

    parent_proc = proc_create("ch4_msg", 0444, parent_proc, &ch4_fops); // proc에 파일 생성

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 >동적 할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &ch4_fops); // cdev구조체 초기화
    ret = cdev_add(cd_cdev, dev_num, 1); // 디바이스 드라이버 삽입
	if (ret < 0) {
		printk("fail to add character device\n");
		return -1;
	}

    return 0;
}

static void __exit ch4_exit(void) { 

	struct list_head *pos = 0;
    struct list_head *q = 0;
	int i = 0;

	printk("ch4: exit module\n");

    cdev_del(cd_cdev); // 디바이스 드라이버 제거
    unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
    proc_remove(parent_proc); // /proc에 만든 파일 제거

	list_for_each_safe(pos, q, &msg_list_head.list) {
    	tmp = list_entry(pos, struct msg_list, list);
        printk("ch4_linked_list: free pos[%d], str[%s]\n", i, tmp->msg->str);
		vfree(tmp->msg);
        kfree(tmp);
        i++;
    }
}

module_init(ch4_init);
module_exit(ch4_exit);
                                              
