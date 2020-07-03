#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해

#define STEP_MOTOR_IOCTL_START_NUM 0x90
#define STEP_MOTOR_IOCTL_NUM1 STEP_MOTOR_IOCTL_START_NUM+1
#define STEP_MOTOR_IOCTL_NUM2 STEP_MOTOR_IOCTL_START_NUM+2
 
#define STEP_MOTOR_IOCTL_NUM 'y'
#define OPEN _IO(STEP_MOTOR_IOCTL_NUM, STEP_MOTOR_IOCTL_NUM1)
#define CLOSE _IO(STEP_MOTOR_IOCTL_NUM, STEP_MOTOR_IOCTL_NUM2)

int step_motor_open() {
	int fd, ret;

	fd = open("/dev/step_motor_dev", O_RDWR);
	if (fd < 0) {
		printf("[step_motor]: open() fail\n");
		return -1;
	}

	ret = ioctl(fd, OPEN);

	return ret;

}

int step_motor_close() {
	int fd, ret;

	fd = open("/dev/step_motor_dev", O_RDWR);
	if (fd < 0) {
		printf("[step_motor]: open() fail\n");
		return -1;
	}

	ret = ioctl(fd, CLOSE);

	return ret;
}
