#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include "simple_block_1.h"

void main(void){
	int fd, ret;
	long value;

	printf("input_value: ");
    scanf("%ld", &value);

    fd = open("/dev/simple_block_1_dev", O_RDWR);
    if (fd < 0) {
    	printf("open() fail\n");
        return;
    }

    ret = ioctl(fd, WQ_WAKE_UP, (unsigned long)value);
    printf("success to produce: %d\n", ret);

    close(fd);
}
