obj-m += target.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

KBUILD_EXTRA_SYMBOLS += $(PWD)/Module.symvers

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

test:
	sudo insmod target.ko
	dmesg