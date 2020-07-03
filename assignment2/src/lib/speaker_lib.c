#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해

#define SPEAKER_IOCTL_START_NUM 0x70
#define SPEAKER_IOCTL_NUM1 SPEAKER_IOCTL_START_NUM+1
#define SPEAKER_IOCTL_NUM2 SPEAKER_IOCTL_START_NUM+2
 
#define SPEAKER_IOCTL_NUM 'x'
#define CORRECT _IO(SPEAKER_IOCTL_NUM, SPEAKER_IOCTL_NUM1)
#define INCORRECT _IO(SPEAKER_IOCTL_NUM, SPEAKER_IOCTL_NUM2)

int speaker_correct() {
	int fd, ret;

	fd = open("/dev/speaker_dev", O_RDWR);
	if (fd < 0) {
		printf("[speaker]: open() fail\n");
		return -1;
	}
	ret = ioctl(fd, CORRECT);

	return ret;
}

int speaker_incorrect() {
	int fd, ret;

	fd = open("/dev/speaker_dev", O_RDWR);
	if (fd < 0) {
		printf("[speaker]: open() fail\n");
		return -1;
	}
	ret = ioctl(fd, INCORRECT);

	return ret;
}
