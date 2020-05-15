obj-m := ch2_mod_201711395.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc -o $(obj-m:mod_201711395.o=app_201711395) $(obj-m:mod_201711395.o=app_201711395.c) 
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm $(obj-m:mod_201711395.o=app_201711395)
