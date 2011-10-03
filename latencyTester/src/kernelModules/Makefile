ifneq ($(KERNELRELEASE),)
KDIR ?= /carme/rootfs/lib/modules/2.6.35.9-xenomai-CARME/build
EXTRA_CFLAGS := -I$(KDIR)/include/xenomai -I$(KDIR)/include/xenomai/rtdm $(ADD_CFLAGS)

obj-m := latencyTester.o latencyTester_kthread.o latencyTester_uthread.o

else
KDIR ?= /carme/rootfs/lib/modules/2.6.35.9-xenomai-CARME/build
PWD := $(shell pwd)

all:
	echo $(CFLAGS)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif
