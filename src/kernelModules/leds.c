/**
 * @brief	LEDs part of HMI device
 * @file    leds.c
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
#include "leds.h"

static struct resource *ledsResource = NULL;
static char *ledsIOBase = NULL;

static ssize_t read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result;

	if (size < 3) {
		return -EFAULT;
	}

	if (*offset == 0) {
		unsigned char value = ioread8(ledsIOBase);

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

static ssize_t write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result;

	unsigned long value = simple_strtoul(buffer, NULL, 0);
	if (!(value >= 0 && value <= 255)) {
		return -ERANGE;
	}
	result = size;

	iowrite8((unsigned char)value, ledsIOBase);

	return result;
}

static struct file_operations ledsFileOperations = {
	.owner = THIS_MODULE,
	.read = read,
	.write = write,
	.llseek = no_llseek
};

static struct miscdevice ledsDevice = {
	.name = "leds",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &ledsFileOperations,
};

static int isLEDsDeviceSetUp = 0;

int __init initLEDs() {
	int result = 0;

	// Request memory region
	if (!(ledsResource = request_mem_region(MMIO_BASE + MMIO_LEDS_OFFSET, 1, "leds"))) {
		result = -EIO;

		goto initLEDs_error;
	}
	// Map memory into kernel virtual address space
	if (!(ledsIOBase = ioremap(MMIO_BASE + MMIO_LEDS_OFFSET, 1))) {
		result = -EIO;

		goto initLEDs_error;
	}

	// Setup character device (based on misc devices)
	if (!(result = misc_register(&ledsDevice))) {
		isLEDsDeviceSetUp = 1;
	} else {
		printk(KERN_WARNING MODULE_LABEL "Error registering LEDs misc device!\n");

		goto initLEDs_error;
	}

	goto initLEDs_out;

initLEDs_error:
initLEDs_out:
	return result;
}

void exitLEDs() {
	if (isLEDsDeviceSetUp) {
		misc_deregister(&ledsDevice);
	}

	if (ledsIOBase) {
		iounmap(ledsIOBase);
	}
	if (ledsResource) {
		release_mem_region(MMIO_BASE + MMIO_LEDS_OFFSET, 1);
	}
}
