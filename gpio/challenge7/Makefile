obj-m := ch7_mod.o

KERNELDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) clean

