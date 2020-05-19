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
#include <linux/string.h>
#include "ku_ipc.h"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define SIMPLE_IOCTL_NUM 'z'
#define KU_IPC_MSGGET _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IPC_MSGCLOSE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long)
#define KU_IPC_MSGSND _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define KU_IPC_MSGRCV _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)

#define DEV_NAME "ku_ipc_dev"
#define QUEUE_NUM 10
#define BUF_SIZE 128

MODULE_LICENSE("GPL");

struct qid {
	int key;
	int msgflg;
};

struct msgbuf {
	long type;
	char text[128];
};

struct msg_config {
	int key;
	struct msgbuf *msg;
	int msgsz;
	long msgtyp;
	int msgflg;
};

struct msg_queue {
	struct list_head list;
	struct msgbuf *msg;
};

static struct msg_queue msg_queue_head[QUEUE_NUM];
struct msg_queue *tmp = 0;
struct msgbuf *kern_buf;
spinlock_t my_lock;
int queue_reference_counter[QUEUE_NUM]; // 생성된 큐 1, 생성되지 않은 큐 0

static long ku_ipc_msgget(struct qid *my_qid) {
	int ret = 0; 

	spin_lock(&my_lock);
	if (my_qid->msgflg == (KU_IPC_CREAT | KU_IPC_EXCL)) {
		if (queue_reference_counter[my_qid->key]) {
			ret = -1;
		}
		else {
			queue_reference_counter[my_qid->key]++;
			ret = my_qid->key;
		}
	}
	else if (my_qid->msgflg == KU_IPC_CREAT){
		queue_reference_counter[my_qid->key]++;
		ret = my_qid->key;
	}
	spin_unlock(&my_lock);

	return ret;
}

static long ku_ipc_msgclose(int msqid) {
	int ret;

	spin_lock(&my_lock);
	if (!queue_reference_counter[msqid] || msqid < 0 || msqid >= QUEUE_NUM) {
		ret = -1;
	}
	else {
		queue_reference_counter[msqid]--;
		ret = 0;
	}
	spin_unlock(&my_lock);

	return ret;
}

static long ku_ipc_msgsnd(struct msg_config *my_msg){
	int ret;
	int msg_num = 0;
	int msg_vol = 0;

	spin_lock(&my_lock);

	list_for_each_entry(tmp, &msg_queue_head[my_msg->key].list, list) {
		msg_num++;
		msg_vol += sizeof(tmp->msg->type) + strlen(tmp->msg->text) + 1;
	}
	msg_vol += sizeof(my_msg->msgsz);

	//printk("queue[%d]의 msg개수: %d, msgsz: %d\n", my_msg->key, msg_num, msg_vol);
	if(msg_vol > KUIPC_MAXVOL || msg_num >= KUIPC_MAXMSG) { // 유휴 공간이 없을때
		//printk("ku_ipc: enter to queue[%d] has no memory\n", my_msg->key);
		ret = -1;
	}
	else { // 유휴 공간이 있을때
		kern_buf = (struct msgbuf *)vmalloc(sizeof(struct msgbuf));
		memset(kern_buf, '\0', sizeof(struct msgbuf));
		ret = copy_from_user(kern_buf, my_msg->msg, my_msg->msgsz);
		tmp = (struct msg_queue *)kmalloc(sizeof(struct msg_queue), GFP_KERNEL);
		tmp->msg = kern_buf;
		list_add_tail(&(tmp->list), &(msg_queue_head[my_msg->key].list));
		//printk("ku_ipc: enter to queue[%d] text[%s]\n", my_msg->key, tmp->msg->text);
	}
	spin_unlock(&my_lock);

	return ret;
}

static long ku_ipc_msgrcv(struct msg_config *my_msg){
	int ret, flag = 0;

	spin_lock(&my_lock);

	list_for_each_entry(tmp, &msg_queue_head[my_msg->key].list, list) {
		//printk("ku_ipc: queue msg type %ld, my_msg type %ld", tmp->msg->type, my_msg->msgtyp);
		if (my_msg->msgtyp == tmp->msg->type) { // 같은 타입의 메세지가 있는 경우
			//printk("ku_ipc: rcv found type %ld in queue[%d]\n", tmp->msg->type, my_msg->key);
			flag = 1;
			if (((my_msg->msgflg & KU_MSG_NOERROR) == 0) && (my_msg->msgsz < sizeof(long) + strlen(tmp->msg->text) + 1)) { // 읽을 수 없는 부분이 있으면
				ret = -2;
				break;
			}
			else { // 읽을 수 있는 만큼 무조건 읽기
				kern_buf = tmp->msg;
				ret = copy_to_user(my_msg->msg, kern_buf, my_msg->msgsz);
				//printk("ku_ipc: delete from queue[%d] text[%s]\n", my_msg->key, tmp->msg->text);
				list_del(&(tmp->list));
				kfree(tmp);
				vfree(kern_buf);
				break;
			}
		}
	}
	if (flag == 0) {
		ret = -1;
	}

	spin_unlock(&my_lock);

	return ret;
}


static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	int ret = 0;
	struct msg_config *my_msg;
	struct qid *my_qid;
	int my_key;

	switch(cmd) {
		case KU_IPC_MSGGET:
			my_qid = (struct qid *)arg;
			ret = ku_ipc_msgget(my_qid);
			//printk("ku_ipc: ioctl msgget return %d\n", ret);
			break;
		case KU_IPC_MSGCLOSE:
			my_key = (int)arg;
			ret = ku_ipc_msgclose(my_key);
			//printk("ku_ipc: ioctl msgclose return %d\n", ret);
			break;
		case KU_IPC_MSGRCV:
			my_msg = (struct msg_config *)arg;
			ret = ku_ipc_msgrcv(my_msg);
			//printk("ku_ipc: ioctl msgrcv return %d\n", ret);
			break;
		case KU_IPC_MSGSND:
			my_msg = (struct msg_config *)arg;
			ret = ku_ipc_msgsnd(my_msg);
			//printk("ku_ipc: ioctl msgsnd return %d\n", ret);
			break;
	}
	return ret;
}

static int ku_ipc_open(struct inode *inode, struct file *file) {
	//printk("ku_ipc: open\n");
	return 0;
}

static int ku_ipc_release(struct inode *inode, struct file *file) {
	//printk("ku_ipc: release\n");
	return 0;
}

struct file_operations ku_ipc_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게된다.
	.open = ku_ipc_open,
	.release = ku_ipc_release,
	.unlocked_ioctl = ku_ipc_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_ipc_init(void){
	int ret, i;
	//printk("ku_ipc: init module\n");

	for(i = 0; i < QUEUE_NUM; i++) { 
		INIT_LIST_HEAD(&(msg_queue_head[i].list));
	}

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 >동적할당후 등록
	cd_cdev = cdev_alloc(); // cdev구조체 초기화
	cdev_init(cd_cdev, &ku_ipc_fops); // cdev구조체 초기화
	ret = cdev_add(cd_cdev, dev_num, 1); // 디바이스 드라이버 삽입
	if (ret < 0) {
		//printk("fail to add character device\n");
		return -1;
	}

	return 0;
}

static void __exit ku_ipc_exit(void) {
	struct list_head *pos = 0;
	struct list_head *q = 0;
	int i = 0;

	//printk("ku_ipc: exit module\n");

	cdev_del(cd_cdev); // 디바이스 드라이버 제거
	unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거

	for (i = 0; i < QUEUE_NUM; i++) {	
		int j = 0;
		list_for_each_safe(pos, q, &msg_queue_head[i].list) {
			tmp = list_entry(pos, struct msg_queue, list);
			//printk("ku_ipc_queue[%d]: free pos[%d], text[%s]\n", i, j, tmp->msg->text);
			vfree(tmp->msg);
			kfree(tmp);
			j++;
		}
	}
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
