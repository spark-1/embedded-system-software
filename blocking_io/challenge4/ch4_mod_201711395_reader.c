#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include <string.h>
#include "ch4.h"

void main(void){
	int fd;
	int ret;

    struct msg_st user_str;

    fd = open("/dev/ch4_dev", O_RDWR);
	if (fd < 0) {
		printf("open() fail\n");
		return;
	}

    ret = ioctl(fd, CH4_IOCTL_READ, &user_str);

	if (ret > 0) {
   		printf("not written %s (ret = %d)\n", user_str.str, ret);
	}
	else {
   		printf("[%d][reader] %s\n", getpid(), user_str.str);
	}

    close(fd);
}
