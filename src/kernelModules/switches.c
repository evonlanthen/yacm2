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

#include <linux/poll.h>

#include <linux/uaccess.h>

#include <linux/io.h>
#include <linux/ioport.h>

#include <asm/atomic.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include <linux/kthread.h>

#include <linux/delay.h>

#include "hmi.h"
#include "helpers.h"
#include "switches.h"

static struct resource *switchesResource = NULL;
static char *switchesIOBase = NULL;

static ssize_t read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result;

	if (*offset == 0) {
		if (size >= 3) {
			unsigned char value = ioread8(switchesIOBase);

			char valueAsString[4];
			//memset(valueAsString, 0, 4);
			//sprintf(valueAsString, "%u", value);
			//size_t valueAsStringLength = strlen(valueAsString);
			size_t valueAsStringLength = toString(valueAsString, 4, value);
			if (!copy_to_user(buffer, valueAsString, valueAsStringLength)) {
				*offset = valueAsStringLength;
				result = valueAsStringLength;
			} else {
				result = /* Error copying data! */ -EFAULT;
			}
		} else {
			result = /* Buffer to short! */ -EFAULT;
		}
	} else {
		result = /* EOF: */ 0;
	}

	return result;
}

static atomic_t isSwitchesEventDeviceAvailable = ATOMIC_INIT(1);

static DEFINE_KFIFO(events, 8);
static spinlock_t eventsLock = SPIN_LOCK_UNLOCKED;

static int openEvent(struct inode *inode, struct file *file) {
	if (!atomic_dec_and_test(&isSwitchesEventDeviceAvailable)) {
	    atomic_inc(&isSwitchesEventDeviceAvailable);
	    return /* Already open: */ -EBUSY;
	}

	// Clear event queue
	spin_lock(&eventsLock);
	kfifo_reset(&events);
	spin_unlock(&eventsLock);

	return 0;
}

static int releaseEvent(struct inode *inode, struct file *file) {
	atomic_inc(&isSwitchesEventDeviceAvailable);

	return 0;
}

static wait_queue_head_t areEventsAvailableWaitQueue;

static int areEventsAvailable(void) {
	spin_lock(&eventsLock);
	if (!kfifo_is_empty(&events)) {
		// Wait condition is true -> keep lock and return
		return 1;
	}
	spin_unlock(&eventsLock);

	return 0;
}

static ssize_t readEvent(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result = 0;

	if (*offset == 0) {
		if (size >= 3) {
			wait_event_interruptible(areEventsAvailableWaitQueue, areEventsAvailable());

			// Remove first event from queue
			unsigned char value = 0;
			if (!kfifo_out(&events, &value, 1)) {
				printk(KERN_WARNING MODULE_LABEL "Event buffer is unexpectedly empty!\n");
			}

			spin_unlock(&eventsLock);

			char valueAsString[4];
			size_t valueAsStringLength = toString(valueAsString, 4, value);
			// Copy data from kernel to user memory
			if (!copy_to_user(buffer, valueAsString, valueAsStringLength)) {
				*offset = valueAsStringLength;
				result = valueAsStringLength;
			} else {
				result = /* Error copying data! */ -EFAULT;
			}
		} else {
			result = /* Buffer to short! */ -EFAULT;
		}
	} else {
		result = /* EOF: */ 0;
	}

	return result;
}

static unsigned int pollEvent(struct file *file, poll_table *wait) {
	unsigned int mask = 0;

	poll_wait(file, &areEventsAvailableWaitQueue, wait);

	if (areEventsAvailable()) {
		mask |= POLLIN | POLLRDNORM;

		spin_unlock(&eventsLock);
	}

	return mask;
}

static unsigned char lastValue = -1;
static struct task_struct * pollSwitchesTask = NULL;

static int pollSwitches(void *unused) {
	while (!kthread_should_stop()) {

		unsigned char value = ioread8(switchesIOBase);
		if (value != lastValue) {
			if (lastValue != -1) {
				// Add new event to queue
				if (!kfifo_in_locked(&events, &value, 1, &eventsLock)) {
					printk(KERN_WARNING MODULE_LABEL "Event buffer is full, new event was ignored!\n");
				}

				wake_up_interruptible_all(&areEventsAvailableWaitQueue);
			}

			lastValue = value;
		}

		if (!kthread_should_stop()) {
			msleep_interruptible(100);
		}
	}

	return 0;
}

static struct file_operations switchesFileOperations = {
	.owner = THIS_MODULE,
	.read = read,
	.llseek = no_llseek,
};

static struct miscdevice switchesDevice = {
	.name = "switches",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &switchesFileOperations,
};

static struct file_operations switchesEventFileOperations = {
	.owner = THIS_MODULE,
	.open = openEvent,
	.read = readEvent,
	.llseek = no_llseek,
	.poll = pollEvent,
	.release = releaseEvent
};

static struct miscdevice switchesEventDevice = {
	.name = "switchesEvent",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &switchesEventFileOperations,
};

static int isSwitchesDeviceSetUp = 0;
static int isSwitchesEventDeviceSetUp = 0;

int __init initSwitches() {
	int result = 0;

	// Request memory region
	if (!(switchesResource = request_mem_region(MMIO_BASE + MMIO_SWITCHES_OFFSET, 1, "switches"))) {
		printk(KERN_WARNING MODULE_LABEL "Error requesting switches I/O memory region!\n");
		result = -EIO;

		goto initSwitches_error;
	}
	// Map memory into kernel virtual address space
	if (!(switchesIOBase = ioremap(MMIO_BASE + MMIO_SWITCHES_OFFSET, 1))) {
		printk(KERN_WARNING MODULE_LABEL "Error mapping switches I/O memory!\n");
		result = -EIO;

		goto initSwitches_error;
	}

	// Setup character devices (based on misc devices)
	if (!(result = misc_register(&switchesDevice))) {
		isSwitchesDeviceSetUp = 1;
	} else {
		printk(KERN_WARNING MODULE_LABEL "Error registering switches misc device!\n");

		goto initSwitches_error;
	}
	if (!(result = misc_register(&switchesEventDevice))) {
		isSwitchesEventDeviceSetUp = 1;
	} else {
		printk(KERN_WARNING MODULE_LABEL "Error registering switches event misc device!\n");

		goto initSwitches_error;
	}

	init_waitqueue_head(&areEventsAvailableWaitQueue);

	// Create and start poll switches task
	pollSwitchesTask = kthread_run(pollSwitches, NULL, "%s", "pollSwitches");
	if (pollSwitchesTask == ERR_PTR(-ENOMEM)) {
		pollSwitchesTask = NULL;
	    result = -ENOMEM;

	    goto initSwitches_error;
	}

	goto initSwitches_out;

initSwitches_error:
initSwitches_out:
	return result;
}

void exitSwitches() {
	if (pollSwitchesTask) {
		kthread_stop(pollSwitchesTask);
	}

	if (isSwitchesEventDeviceSetUp) {
		misc_deregister(&switchesEventDevice);
	}
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
