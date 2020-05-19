#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include "simple_block_2.h"

void main(void){
	int fd, ret, cmd;
	long value;

	printf("[%d] choose cmd: ", getpid());
	scanf("%d", &cmd);
	printf("[%d] choose value: ", getpid());
	scanf("%ld", &value);

	fd = open("/dev/simple_block_2_dev", O_RDWR);
	if (fd < 0) {
		printf("open() fail\n");
		return;
	}

	switch(cmd) {
		case 1:
			ret = ioctl(fd, WQ_WAKE_UP, (unsigned long)value);
			break;
		case 2:
			ret = ioctl(fd, WQ_WAKE_UP_ALL, (unsigned long)value);
			break;
	}

	printf("success to produce: %d\n", ret);

	close(fd);
}
