#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  // 파일구조체를 위해 inode 
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "ch2_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define MAGIC_NUM 'z'
#define GET_IOCTL _IOWR(MAGIC_NUM, IOCTL_NUM1, unsigned long)
#define SET_IOCTL _IOWR(MAGIC_NUM, IOCTL_NUM2, unsigned long)
#define ADD_IOCTL _IOWR(MAGIC_NUM, IOCTL_NUM3, unsigned long)
#define MUL_IOCTL _IOWR(MAGIC_NUM, IOCTL_NUM4, unsigned long)

static long result = 0;
module_param(result, long, 0);

static int ch2_open(struct inode *inode, struct file *file) {
        printk("ch2: open\n");
        return 0;
}

static int ch2_release(struct inode *inode, struct file *file) {
        printk("ch2: release\n");
        return 0;
}

static long ch2_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

        switch(cmd) {
                case GET_IOCTL:
                        printk("ch2: return result %ld \n", result);
                        return result;
                case SET_IOCTL:
                        printk("ch2: set result %ld to %ld \n", result, (unsigned long)arg);
                        result = (unsigned long)arg;
                        return result;
                case ADD_IOCTL:
                        printk("ch2: add result %ld + %ld \n", result, (unsigned long)arg);
                        result += (unsigned long)arg;
                        return result;
                case MUL_IOCTL:
                        printk("ch2: mul result %ld x %ld \n", result, (unsigned long)arg);
                        result *= (unsigned long)arg;
                        return result;
                default:
                        return -1;
        }
        return 0;
}

struct file_operations ch2_fops = { // 초기화 하지 않은 다른 fops 함수들은 null값을 갖게된다
        .open = ch2_open,
        .release = ch2_release,
        .unlocked_ioctl = ch2_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch2_init(void) {
        printk("ch2: init module\n");

        alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); // /proc/devices 에서 확인후 메이저번호 동적 할당후 등록
        cd_cdev = cdev_alloc(); // cdev구조체 초기화
        cdev_init(cd_cdev, &ch2_fops); // cdev구조체 초기화
        cdev_add(cd_cdev, dev_num, 1); // /dev 에 디바이스 드라이버 삽입

        return 0;
}

static void __exit ch2_exit(void) {
        printk("ch2:exit module\n");

        cdev_del(cd_cdev); // /dev 에 디바이스 드라이버 제거
        unregister_chrdev_region(dev_num, 1); // /proc/devices 에서 해당 다바이스 드라이브 제거
}

module_init(ch2_init);
module_exit(ch2_exit);

