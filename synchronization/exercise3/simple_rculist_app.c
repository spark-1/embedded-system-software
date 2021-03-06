#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해

#define READ 1
#define UPDATE 2

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_READ _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_UPDATE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

int main(int argc, char *argv[]){
	int dev;
	int i, id, op;

	if(argc != 3) {
		printf("insert two arguments\n");
		printf("first argument = (1 : read, 2 : write)\n");
		printf("second argument = number of iterations\n");
		return -1;
	}

	op = atoi(argv[1]);
	id = atoi(argv[2]);

	dev = open("/dev/simple_rculist_dev", O_RDWR);

	if (dev < 0) {
		printf("open() fail\n");
		return -1;
	}

	if(op == READ) {
		ioctl(dev, IOCTL_READ, (unsigned long) id);
	}
	else if (op == UPDATE) {
		ioctl(dev, IOCTL_UPDATE, (unsigned long) id);
	}
	else {
		printf("invalid operation\n");
		close(dev);
		return -1;
	}

	close(dev);
	return 0;
}

