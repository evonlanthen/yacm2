/**
 * @brief	Switches part of HMI device
 * @file    switches.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#include <linux/kernel.h>

#include <linux/miscdevice.h>

#include <linux/fs.h>

#include <asm/uaccess.h>

#include <linux/io.h>
#include <linux/ioport.h>

#include "hmi.h"
#include "switches.h"

static struct resource *switchesResource = NULL;
static char *switchesIOBase = NULL;

static ssize_t read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result;

	if (size < 3) {
		return -EFAULT;
	}

	if (*offset == 0) {
		unsigned char value = ioread8(switchesIOBase);

		char valueAsString[4];
		memset(valueAsString, 0, 4);
		sprintf(valueAsString, "%u", value);
		size_t valueAsStringLength = strlen(valueAsString);
		if (copy_to_user(buffer, valueAsString, valueAsStringLength)) {
			result = -EFAULT;
		} else {
			*offset = valueAsStringLength;
			result = valueAsStringLength;
		}
	} else {
		result = /* EOF: */ 0;
	}

	return result;
}

static struct file_operations switchesFileOperations = {
	.owner = THIS_MODULE,
	.read = read,
	.llseek = no_llseek
};

static struct miscdevice switchesDevice = {
	.name = "switches",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &switchesFileOperations,
};

static int isSwitchesDeviceSetUp = 0;

int __init initSwitches() {
	int result = 0;

	// Request memory region
	if (!(switchesResource = request_mem_region(MMIO_BASE + MMIO_SWITCHES_OFFSET, 1, "switches"))) {
		result = -EIO;

		goto initSwitches_error;
	}
	// Map memory into kernel virtual address space
	if (!(switchesIOBase = ioremap(MMIO_BASE + MMIO_SWITCHES_OFFSET, 1))) {
		result = -EIO;

		goto initSwitches_error;
	}

	// Setup character device (based on misc devices)
	if (!(result = misc_register(&switchesDevice))) {
		isSwitchesDeviceSetUp = 1;
	} else {
		printk(KERN_WARNING MODULE_LABEL "Error registering switches misc device!\n");

		goto initSwitches_error;
	}

	goto initSwitches_out;

initSwitches_error:
initSwitches_out:
	return result;
}

void exitSwitches() {
	if (isSwitchesDeviceSetUp) {
		misc_deregister(&switchesDevice);
	}

	if (switchesIOBase) {
		iounmap(switchesIOBase);
	}
	if (switchesResource) {
		release_mem_region(MMIO_BASE + MMIO_SWITCHES_OFFSET, 1);
	}
}
