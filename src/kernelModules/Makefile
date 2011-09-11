ifneq ($(KERNELRELEASE),)
obj-m := hello.o stroboscope.o

else
KDIR ?= /carme/kernel/linux-2.6.35.9-adeos-ipipe-1.18-01-CARME 
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
endif

