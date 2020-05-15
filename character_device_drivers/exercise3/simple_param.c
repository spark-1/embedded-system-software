#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_param_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define PARAM_IOCTL_NUM 'z'
#define PARAM_GET _IOWR(PARAM_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define PARAM_SET _IOWR(PARAM_IOCTL_NUM, IOCTL_NUM2, unsigned long)

static long my_id = 0;
module_param(my_id, long, 0);

static int simple_param_open(struct inode *inode, struct file *file) {
        printk("simple_param: open\n");
        return 0;
}

static int simple_param_release(struct inode *inode, struct file *file) {
        printk("simple_param: release\n");
        return 0;
}

static long simple_param_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

        switch(cmd) {
                case PARAM_GET:
                        printk("simple_param: return my_id %ld \n", my_id);
                        return my_id;
                case PARAM_SET:
                        printk("simple_param: set my_id %ld to %ld \n", my_id, (long)arg);
                        my_id = (long)arg;
						return my_id;
                default:
                        return -1;
        }
        return 0;
}

struct file_operations simple_param_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게 >된다
        .open = simple_param_open,
        .release = simple_param_release,
        .unlocked_ioctl = simple_param_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_param_init(void) {
        printk("simple_param: init module\n");

        alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 동적 할당후 등록
        cd_cdev = cdev_alloc(); // cdev구조체 초기화
        cdev_init(cd_cdev, &simple_param_fops); // cdev구조체 초기화
        cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입

        return 0;
}

static void __exit simple_param_exit(void) {
        printk("simple_param :exit module\n");

        cdev_del(cd_cdev); // /dev 에 디바이스 드라이버 제거
        unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
}

module_init(simple_param_init);
module_exit(simple_param_exit);

