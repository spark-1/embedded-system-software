#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include <string.h>
#include "ch3.h"

void main(void){
	int dev, ret;

	struct msg_st user_str = {
    	0, ""
    };

	printf("[writer]: ");
	scanf("%s", user_str.str);
    user_str.len = strlen(user_str.str);

    dev = open("/dev/ch3_dev", O_RDWR);
	if (dev < 0) {
		printf("open() fail\n");
		return;
	}

    ret = ioctl(dev, CH3_IOCTL_WRITE, &user_str);
	if (ret < 0)
		printf("ioctl() 실패\n");

    close(dev);
}
