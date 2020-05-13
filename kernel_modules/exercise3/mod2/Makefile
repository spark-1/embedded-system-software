obj-m := mod2.o

KBUILD_EXTRA_SYMBOLS := /home/spark/임소/exercise3/mod1/Module.symvers
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
debug:
	$(MAKE) EXTRA_CFLAGS=-DMY_DEBUG -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
