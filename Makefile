################################################################################
# Makefile for yacm 
################################################################################

# Directory of root filesystem
ROOTFS		= /carme/rootfs

# Build settings
KDIR            = /carme/kernel/linux-2.6.35.9-adeos-ipipe-1.18-01-CARME
CC		= arm-linux-gcc
CFLAGS		= -Wall -std=c99 -I$(ROOTFS)/usr/include -I$(ROOTFS)/usr/include/microwin -D DEBUG -D_BSD_SOURCE
LDFLAGS 	= -lrt -lpthread -lnano-X -lvncserver -lm -lpng -lfreetype -ljpeg -lz -lSDL -lSDL_mixer -ldirectfb -ldirect -lfusion -lmad -L$(ROOTFS)/usr/lib

# Installation variables
EXEC_NAME	= yacm

# Make rules
all: carme modules carme-install

orchid:
	$(CC) $(CFLAGS) -o $(EXEC_NAME)_orchid src/*.c $(LDFLAGS)

carme:
	$(CC) -DCARME $(CFLAGS) -o $(EXEC_NAME)_carme src/*.c $(LDFLAGS)

modules:
	$(MAKE) -C src/kernelModules

clean:
	$(RM) *.o $(EXEC_NAME)_* $(EXEC_NAME)

orchid-install:
	cp $(EXEC_NAME)_orchid $(ROOTFS)/usr/local/bin/$(EXEC_NAME)

carme-install:
	cp $(EXEC_NAME)_carme $(ROOTFS)/usr/local/bin/$(EXEC_NAME)
	cp $(EXEC_NAME)_carme $(ROOTFS)/root/$(EXEC_NAME)
	
install: carme-install
	#@echo "Please use 'make orchid-install' or 'make carme-install'"

doc:
	doxygen

.PHONY:	doc
