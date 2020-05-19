#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include <string.h>
#include "ch4.h"

void main(void){
	int fd, ret;

	struct msg_st user_str = {
    	0, ""
    };

	printf("[%d][writer]: ", getpid());
	scanf("%s", user_str.str);
    user_str.len = strlen(user_str.str);

    fd = open("/dev/ch4_dev", O_RDWR);
	if (fd < 0) {
		printf("open() fail\n");
		return;
	}

    ret = ioctl(fd, CH4_IOCTL_WRITE, &user_str);
	if (ret != 0) {
		printf("fail to write %s (ret = %d)\n", user_str.str, ret);
	}
	else {
		printf("success to write %s\n", user_str.str);
	}

    close(fd);
}
