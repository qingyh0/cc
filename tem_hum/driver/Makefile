name ?= demo
arch ?= arm

ifeq ($(arch),x86)
KERNELDIR := /lib/modules/$(shell uname -r)/build/
else
KERNELDIR := /home/yan/linux-5.10.10/
endif

PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules
	#-C进入内核目录，使用内核目录的makefile文件
	#M=制定编译模块的路径

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

install:
	cp gpio.ko ~/nfs/rootfs/armc/
	cp m74hc595.ko ~/nfs/rootfs/armc/
	cp si7006.ko ~/nfs/rootfs/armc/

# obj-m := $(name).o
obj-m := gpio.o
obj-m += m74hc595.o
obj-m += si7006.o
