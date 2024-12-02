KVERS = $(shell uname -r)
KERNEL_DIR := /usr/src/linux-headers-$(KVERS)

CURRENT_PATH := $(shell pwd)

obj-m += globalfifo.o

# 开启调试功能
EXTRA_CFLAGS=-g -O0

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_PATH) clean

install:
	sudo insmod globalfifo.ko
	sudo mknod /dev/globalfifo c 230 0
	sudo chmod 777 /dev/globalfifo

uninstall:
	sudo rmmod globalfifo.ko
	sudo rm /dev/globalfifo