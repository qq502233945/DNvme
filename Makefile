
obj-m += dnvme.o

KERN_DIR := /home/qs/linuxs/linux
KERN_DIR += /home/qs/linuxs/linux/drivers
all:
	make -C $(KERN_DIR) M=$(PWD) modules
clean:
	make -C $(KERN_DIR) M=$(PWD) 

test:
	sudo dmesg -C
	sudo insmod dnvme.ko
	sudo rmmod dnvme.ko
	dmesg