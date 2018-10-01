obj-m := lba_check.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
