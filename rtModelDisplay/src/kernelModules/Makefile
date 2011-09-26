ifneq ($(KERNELRELEASE),)
KDIR ?= /carme/kernel/linux-2.6.35.9-adeos-ipipe-1.18-01-CARME
EXTRA_CFLAGS := -I$(KDIR)/include/xenomai -I$(KDIR)/include/xenomai/rtdm $(ADD_CFLAGS)

obj-m := display.o

else
KDIR ?= /carme/kernel/linux-2.6.35.9-adeos-ipipe-1.18-01-CARME
PWD := $(shell pwd)

all:
	echo $(CFLAGS)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif
