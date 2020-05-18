#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해
#include <string.h>
#include "simple_spin.h"

void main(void){
        int dev;

        struct str_st user_str = {
		0, "reader: hi, writer!"
	};

	user_str.len = strlen(user_str.str);
		
	printf("Writer: Hello, reader!\n");

        dev = open("/dev/simple_spin_dev", O_RDWR);
	if(dev <= 0) {
		printf("open 실패\n");
	}

        ioctl(dev, SIMPLE_SPIN_WRITE, &user_str);
        
        close(dev);
}

