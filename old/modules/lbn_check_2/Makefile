obj-m += lbn_check_2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
