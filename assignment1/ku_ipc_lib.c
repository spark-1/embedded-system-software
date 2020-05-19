#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include <string.h>
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

#define QUEUE_NUM 10

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

int queue_use[10]; // 해당 큐를 사용하고 있는지 여부, 같은 큐를 msgget 방지

int ku_msgget(int key, int msgflg) { // key: 0~9 
	int fd, ret;
	struct qid my_qid;

	if (queue_use[key]){
    	//printf("[ku_msgget]: fail, queue[%d] is already using by process\n", key);
		return -1;
	}

	fd = open("/dev/ku_ipc_dev", O_RDWR);
    if (fd < 0) {
    	//printf("[ku_msgget]: open() fail\n");
        return -1;
    }

	my_qid.key = key;
	my_qid.msgflg = msgflg;
    ret = ioctl(fd, KU_IPC_MSGGET, &my_qid);

    if (ret < 0) {
    	//printf("[ku_msgget]: fail, queue[%d] is already using by other process\n", key);
    }
    else {
        //printf("[ku_msgget]: success, access queue[%d]\n", ret);
		queue_use[ret] = 1;
    }

    close(fd);
	return ret;
}

int ku_msgclose(int msqid){
	int fd, ret;
    
	if (!queue_use[msqid]){
    	//printf("[ku_msgclose]: fail, msgget queue[%d] first\n", msqid);
		return -1;
	}

	fd = open("/dev/ku_ipc_dev", O_RDWR);
    if (fd < 0) {
    	//printf("[ku_msgclose]: open() fail\n");
        return -1;
    }

    ret = ioctl(fd, KU_IPC_MSGCLOSE, msqid);
    if (ret < 0) {
    	//printf("[ku_msgclose]: fail, cannot close queue[%d]\n", msqid);
    }
    else {
        //printf("[ku_msgclose]: success, close queue[%d]\n", msqid);
		queue_use[msqid] = 0;
    }

    close(fd);

	return ret;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg){
	int fd, ret;
	struct msg_config my_msg;

	if (!queue_use[msqid]){
    	//printf("[ku_msgsnd]: fail, msgget queue[%d] first\n", msqid);
		return -1;
	}

	fd = open("/dev/ku_ipc_dev", O_RDWR);
    if (fd < 0) {
    	//printf("[ku_msgsnd]: open() fail\n");
        return -1;
    }

	my_msg.key = msqid;
	my_msg.msg = msgp;
	my_msg.msgsz = msgsz;
	my_msg.msgflg = msgflg;

    ret = ioctl(fd, KU_IPC_MSGSND, &my_msg);

	if ((msgflg & KU_IPC_NOWAIT) != 0) {
    	if (ret < 0) {
    		//printf("[ku_msgsnd]: fail, queue[%d] is full\n", msqid);
    	}	
    	else {
        	//printf("[ku_msgsnd] success, queue[%d] get msg\n", msqid);
    	}
	}
	else if ((msgflg & KU_IPC_NOWAIT) == 0) {
		while (ret < 0) {
    		ret = ioctl(fd, KU_IPC_MSGSND, &my_msg);
		}
        //printf("[ku_msgsnd] success, queue[%d] get msg\n", msqid);
	}

    close(fd);

	return ret;
}

int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg){
	int fd, ret;
	struct msg_config my_msg;

	if (!queue_use[msqid]){
    	//printf("[ku_msgsnd]: fail, msgget queue[%d] first\n", msqid);
		return -1;
	}

	fd = open("/dev/ku_ipc_dev", O_RDWR);
    if (fd < 0) {
    	//printf("[ku_msg]: open() fail\n");
        return -1;
    }

	my_msg.key = msqid;
	my_msg.msg = msgp;
	my_msg.msgsz = msgsz;
	my_msg.msgtyp = msgtyp;
	my_msg.msgflg = msgflg;

    ret = ioctl(fd, KU_IPC_MSGRCV, &my_msg);

	if ((msgflg & KU_IPC_NOWAIT) != 0) {
    	if (ret == -1) { // 메세지가 없어서 못받은 경우
    		//printf("[ku_msgrcv]: fail, queue[%d] has no %ld type msg\n", msqid, msgtyp);
    	}
		else if (ret == -2) { // 메세지는 있으나 msgsz에서 걸려 못받은 경우
    		//printf("[ku_msgrcv]: fail, msgsz is too small\n");
			ret = -1;
		}
    	else {
			ret = sizeof(long) + strlen(my_msg.msg->text) + 1;
        	//printf("[ku_msgrcv] success, received msg from queue[%d]\n", msqid);
    	}
	}
	else if ((msgflg & KU_IPC_NOWAIT) == 0) {
		while (ret == -1) { // 메세지가 없어서 못받은 경우
    		ret = ioctl(fd, KU_IPC_MSGRCV, &my_msg);
		}
		if (ret == -2) { // 메세지는 있으나 msgsz에서 걸려 못받은 경우
    		//printf("[ku_msgrcv]: fail, msgsz is too small\n");
			ret = -1;
		}
		else {
			ret = sizeof(long) + strlen(my_msg.msg->text) + 1;
        	//printf("[ku_msgrcv] success, received msg from queue[%d]\n", msqid);
		}
	}

    close(fd);

	return ret;
}
