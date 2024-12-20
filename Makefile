KVERS = $(shell uname -r)
KERNEL_DIR := /usr/src/linux-headers-$(KVERS)

CURRENT_PATH := $(shell pwd)

obj-m += hello.o

# 开启调试功能
EXTRA_CFLAGS=-g -O0

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_PATH) clean

install:
	sudo insmod hello.ko

uninstall:
	sudo rmmod hello.ko