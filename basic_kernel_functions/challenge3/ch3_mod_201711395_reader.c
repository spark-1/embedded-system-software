#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include <string.h>
#include "ch3.h"

void main(void){
	int dev;
	int ret;

    struct msg_st user_str;

    dev = open("/dev/ch3_dev", O_RDWR);
	if (dev < 0) {
		printf("open() fail\n");
		return;
	}

    ret = ioctl(dev, CH3_IOCTL_READ, &user_str);

    if (ret < 0) {
	    printf("[reader]: nothing in kernel msg list\n");
	}
    else {
    	printf("[reader] %s\n", user_str.str);
	}

    close(dev);
}
