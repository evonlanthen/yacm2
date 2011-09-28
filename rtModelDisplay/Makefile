################################################################################
# Makefile for yacm-rt-model-display
################################################################################

# Directory of root filesystem
ROOTFS		= /carme/rootfs
INSTALL_DIR	= root

# Build settings
# Kernel
#KDIR		= /carme/kernel/linux-2.6.35.9-adeos-ipipe-1.18-01-CARME

# Xenomai
DESTDIR		= $(ROOTFS)
XENO		= $(ROOTFS)/usr/xenomai
XENOCONFIG	= $(shell PATH=$(XENO):$(XENO)/bin:$(PATH) which xeno-config 2>/dev/null)

#CC		= arm-linux-gcc
CC		= $(shell $(XENOCONFIG) --cc)
MY_CFLAGS	= -Wall -D DEBUG -D_BSD_SOURCE -I../src
# -std=c99
# -I/carme/rootfs/usr/include -I/carme/rootfs/usr/include/microwin
MY_LDFLAGS 	= ../device.o ../log.o
# -lrt -lpthread
# -lnano-X -lvncserver -lm -lpng -lfreetype -ljpeg -lz -lSDL -lSDL_mixer -ldirectfb -ldirect -lfusion -lmad -L$(ROOTFS)/usr/lib ../device.o ../log.o

CFLAGS=$(shell XENO=$(XENO) DESTDIR=$(DESTDIR) $(XENOCONFIG) --skin=posix --cflags) $(MY_CFLAGS)
LDFLAGS=$(shell XENO=$(XENO) DESTDIR=$(DESTDIR) $(XENOCONFIG) --skin=posix --ldflags) $(MY_LDFLAGS)
LDFLAGS+=-Xlinker -rpath -Xlinker $(shell $(XENOCONFIG) --libdir)

# Installation variables
EXEC_NAME	= yacm-rt-model-display

# Make rules
all: carme modules carme-install

orchid:
	$(CC) $(CFLAGS) -o $(EXEC_NAME)_orchid ../src/activity.c src/*.c $(LDFLAGS)

carme:
	$(CC) -DCARME $(CFLAGS) -o $(EXEC_NAME)_carme ../src/activity.c src/*.c $(LDFLAGS)

modules:
	$(MAKE) -C src/kernelModules

clean: modules-clean
	$(RM) *.o $(EXEC_NAME)_* $(EXEC_NAME)

modules-clean:
	$(MAKE) -C src/kernelModules clean

orchid-install:
	sudo cp $(EXEC_NAME)_orchid $(ROOTFS)/usr/local/bin/$(EXEC_NAME)

carme-install:
	sudo cp $(EXEC_NAME)_carme $(ROOTFS)/$(INSTALL_DIR)/$(EXEC_NAME)
	sudo cp src/kernelModules/*.ko $(ROOTFS)/$(INSTALL_DIR)/

install: carme-install

doc:
	doxygen

.PHONY:	doc
