CROSS_COMPILE ?=
ARCH ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

obj-m := test.o
test-objs := main.o ioctl/miscdev.o func/copy_data.o

EXTRA_CFLAGS += -I$(src)

PWD = $(shell pwd)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
CONFIG_MODULE_SIG = n

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
