ifneq ($(KERNELRELEASE),)
obj-m := hmi.o grinder.o
hmi-objs := buttons.o switches.o leds.o helpers.o

else
KDIR ?= /carme/kernel/linux-2.6.35.9-adeos-ipipe-1.18-01-CARME 
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	cp hmi.ko grinder.ko /carme/rootfs/root

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif

