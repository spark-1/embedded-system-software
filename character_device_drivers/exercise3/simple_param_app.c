#include <stdio.h>
#include <sys/fcntl.h> // open()을 위해
#include <unistd.h> // close()를 위해
#include <sys/ioctl.h> // ioctl()을 위해

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define PARAM_IOCTL_NUM 'z'
#define PARAM_GET _IOWR(PARAM_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define PARAM_SET _IOWR(PARAM_IOCTL_NUM, IOCTL_NUM2, unsigned long)

void main(void){
        int dev;
        long my_id;

        dev = open("/dev/simple_param_dev", O_RDWR);

	my_id = 201711395;
        ioctl(dev, PARAM_SET, (unsigned long)my_id);
        ioctl(dev, PARAM_GET, NULL);
	printf("My id is %ld \n", my_id);

        close(dev);
}       

