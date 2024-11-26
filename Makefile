KERNEL_DIR := /usr/src/linux-headers-6.6.20+rpt-rpi-v8

CURRENT_PATH := $(shell pwd)

obj-m := hello.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURRENT_PATH) clean
