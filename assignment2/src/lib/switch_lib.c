#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해

#define SWITCH_IOCTL_START_NUM 0x80
#define SWITCH_IOCTL_NUM1 SWITCH_IOCTL_START_NUM+1
#define SWITCH_IOCTL_NUM2 SWITCH_IOCTL_START_NUM+2
#define SWITCH_IOCTL_NUM3 SWITCH_IOCTL_START_NUM+3
#define SWITCH_IOCTL_NUM4 SWITCH_IOCTL_START_NUM+4

#define SWITCH_IOCTL_NUM 'z'
#define SWITCH_QUEUE_GET _IOWR(SWITCH_IOCTL_NUM, SWITCH_IOCTL_NUM1, unsigned long)
#define SWITCH_QUEUE_CLOSE _IOWR(SWITCH_IOCTL_NUM, SWITCH_IOCTL_NUM2, unsigned long)
#define SWITCH_RECV _IOWR(SWITCH_IOCTL_NUM, SWITCH_IOCTL_NUM3, unsigned long)
#define ULTRASONIC_RECV _IOWR(SWITCH_IOCTL_NUM, SWITCH_IOCTL_NUM4, unsigned long)

int switch_queue_get(int key) { // key: 0~9 
	int fd, ret;

	fd = open("/dev/switch_dev", O_RDWR);
    if (fd < 0) {
    	printf("[switch_queue_get]: open() fail\n");
        return -1;
    }

    ret = ioctl(fd, SWITCH_QUEUE_GET, key);
    if (ret < 0) {
    	printf("[switch_queue_get]: fail, queue[%d] is already using by other process\n", key);
    }
    else {
        printf("[switch_queue_get]: success, access queue[%d]\n", key);
    }

    close(fd);
	return ret;
}

int switch_queue_close(int key){
	int fd, ret;

	fd = open("/dev/switch_dev", O_RDWR);
    if (fd < 0) {
    	printf("[switch_queue_close]: open() fail\n");
        return -1;
    }

    ret = ioctl(fd, SWITCH_QUEUE_CLOSE, key);
    if (ret < 0) {
    	printf("[switch_queue_close]: fail, cannot close queue[%d] get it first\n", key);
    }
    else {
        printf("[switch_queue_close]: success, close queue[%d]\n", key);
    }

    close(fd);
	return ret;
}

int switch_recv(int key) { // 내용이 없으면 -1 있으면 1,2,3중 하나 
	int fd, ret;

	fd = open("/dev/switch_dev", O_RDWR);
    if (fd < 0) {
    	printf("[switch_recv]: open() fail\n");
        return -1;
    }

    ret = ioctl(fd, SWITCH_RECV, key);
	if (ret < 0) {
   		//printf("[switch_recv]: fail, queue[%d] has no value\n", key);
	}
	else {
        //printf("[switch_recv] success, received value %d from queue[%d]\n", ret, key);
	}

    close(fd);
	return ret;
}

int ultrasonic_recv(int key) { // 내용이 없으면 -1 있으면 거리 리턴
	int fd, ret;

	fd = open("/dev/switch_dev", O_RDWR);
    if (fd < 0) {
    	printf("[ultrasonic_recv]: open() fail\n");
        return -1;
    }

    ret = ioctl(fd, ULTRASONIC_RECV, key);
	if (ret < 0) {
   		//printf("[ultrasonic_recv]: fail, queue[%d] has no value\n", key);
	}
	else {
        //printf("[ultrasonic_recv] success, received value %d from queue[%d]\n", ret, key);
	}

    close(fd);
	return ret;
}
