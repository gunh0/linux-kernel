obj-m += belt.o

PWD := $(shell pwd)
 
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	cp belt.ko /lib/modules/$(shell uname -r)/kernel/crypto/
	depmod -a
	modprobe belt