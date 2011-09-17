/**
 * @brief	HMI device
 * @file    hmi.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/miscdevice.h>

#include <linux/fs.h>

#include <linux/uaccess.h>

#include <linux/gpio.h>

#include <linux/interrupt.h>

#include <asm/atomic.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include "hmi.h"
#include "switches.h"
#include "leds.h"
#include "helpers.h"

static struct gpio buttonGPIOs[] = {
	{ 99, GPIOF_IN, "T0" },
	{ 101, GPIOF_IN, "T1" },
	{ 102, GPIOF_IN, "T2" },
	{ 103, GPIOF_IN, "T3" }
};

static struct timer_list debounceButtonTimers[4];
static volatile unsigned long debounceButtonFlags = 0;

static atomic_t isButtonsDeviceAvailable = ATOMIC_INIT(1);

static DEFINE_KFIFO(buttonEvents, 8);
static spinlock_t eventsLock = SPIN_LOCK_UNLOCKED;

static int open(struct inode *inode, struct file *file) {
	if (!atomic_dec_and_test(&isButtonsDeviceAvailable)) {
	    atomic_inc(&isButtonsDeviceAvailable);
	    return /* Already open: */ -EBUSY;
	}

	// Clear event queue
	static unsigned long interruptFlags;
	spin_lock_irqsave(&eventsLock, interruptFlags);
	kfifo_reset(&buttonEvents);
	spin_unlock_irqrestore(&eventsLock, interruptFlags);

	return 0;
}

static int release(struct inode *inode, struct file *file) {
	atomic_inc(&isButtonsDeviceAvailable);

	return 0;
}

static wait_queue_head_t areEventsAvailableWaitQueue;

static unsigned long interruptFlags;

static int areEventsAvailable(void) {
	// Open atomic context (by disabling interrupts)
	spin_lock_irqsave(&eventsLock, interruptFlags);
	if (!kfifo_is_empty(&buttonEvents)) {
		// Wait condition is true -> keep lock and return
		return 1;
	}
	spin_unlock_irqrestore(&eventsLock, interruptFlags);

	return 0;
}

static ssize_t read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result;

	if (*offset == 0) {
		if (size >= 1) {
			wait_event_interruptible(areEventsAvailableWaitQueue, areEventsAvailable());

			// Remove first event from queue
			unsigned char value = 0;
			if (!kfifo_out(&buttonEvents, &value, 1)) {
				printk(KERN_WARNING MODULE_LABEL "Event buffer is unexpectedly empty!\n");
			}

			spin_unlock_irqrestore(&eventsLock, interruptFlags);

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

static struct file_operations buttonsFileOperations = {
	.owner = THIS_MODULE,
	.open = open,
	.read = read,
	.release = release,
	.llseek = no_llseek
};

static struct miscdevice buttonsDevice = {
	.name = "buttonsEvent",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &buttonsFileOperations,
};

static irqreturn_t handleButtonInterrupt(int irq, void *deviceId) {
	int buttonIndex = (int)deviceId;

	//printk(KERN_INFO MODULE_LABEL "Interrupt for button %d (button GPIO %d) occured!\n", index, buttonGPIOs[index].gpio);

	if (test_and_set_bit(buttonIndex, &debounceButtonFlags)) {
		//printk(KERN_INFO MODULE_LABEL "Ignoring interrupt for GPIO %d.\n", buttonGPIOs[index].gpio);

		return IRQ_HANDLED;
	}

	mod_timer(&debounceButtonTimers[buttonIndex], jiffies + msecs_to_jiffies(100));

	// Add new event to queue
	if (!kfifo_in_locked(&buttonEvents, &buttonIndex, 1, &eventsLock)) {
		printk(KERN_WARNING MODULE_LABEL "Event buffer is full, new event was ignored!\n");
	}

	wake_up_interruptible_all(&areEventsAvailableWaitQueue);

	return IRQ_HANDLED;
	//return IRQ_WAKE_THREAD;
}

//static irqreturn_t handleButtonInterrupt2ndStage(int irq, void *deviceId) {
	//int buttonIndex = (int)deviceId;

	//return IRQ_HANDLED;
//}

static void buttonDebounceTimerExpired(unsigned long data) {
	//printk(KERN_INFO MODULE_LABEL "Debounce timer for button GPIO %d expired.\n", buttonGPIOs[data].gpio);

	clear_bit(data, &debounceButtonFlags);
}

static int isButtonsDeviceSetUp = 0;

void __exit exitHMI(void);
int __init initHMI(void) {
	int result = 0;

	printk(KERN_INFO MODULE_LABEL "Loading module...\n");

	// 1. Setup button device
	// 1.1 Clear alternate GPIO functions
	// ... Nothing to do ...

	// 1.2 Claim GPIOs
	if ((result = gpio_request_array(buttonGPIOs, ARRAY_SIZE(buttonGPIOs)))) {
		printk(KERN_WARNING MODULE_LABEL "Error requesting buttons GPIO array!\n");

		goto initHMI_error;
	}

	// 1.3 Initialize debounce timers
	int i;
	for (i = 0; i < ARRAY_SIZE(debounceButtonTimers); i++) {
		init_timer(&debounceButtonTimers[i]);
		debounceButtonTimers[i].function = buttonDebounceTimerExpired;
		debounceButtonTimers[i].data = i;
	}

	// 1.4 Request interrupts
	for (i = 0; i < ARRAY_SIZE(buttonGPIOs); i++) {
		int buttonGPIOInterrupt = gpio_to_irq(buttonGPIOs[i].gpio);

		printk(KERN_INFO MODULE_LABEL "Button GPIO %d interrupt: %d\n", buttonGPIOs[i].gpio, buttonGPIOInterrupt);

		if ((result = request_irq /* request_threaded_irq */ (buttonGPIOInterrupt,
								handleButtonInterrupt,
								//handleButtonInterrupt2ndStage,
								IRQF_TRIGGER_RISING,
								buttonGPIOs[i].label,
								/* Button index */ (void *)i))) {
			printk(KERN_WARNING MODULE_LABEL "Error requesting buttons GPIO interrupts!\n");

			goto initHMI_error;
		}
	}

	// 1.5 Initialize wait queue (used for signaling to waiting read system calls that data is available)
	init_waitqueue_head(&areEventsAvailableWaitQueue);

	// 1.6 Setup character device (based on misc devices)
	if (!(result = misc_register(&buttonsDevice))) {
		isButtonsDeviceSetUp = 1;
	} else {
		printk(KERN_WARNING MODULE_LABEL "Error registering buttons misc device!\n");

		goto initHMI_error;
	}

	// 2. Setup switches device
	if ((result = initSwitches()) < 0) {
		goto initHMI_error;
	}

	// 3. Setup LEDs device
	if ((result = initLEDs()) < 0) {
		goto initHMI_error;
	}

	printk(KERN_INFO MODULE_LABEL "...done. (loading)\n");

	goto initHMI_out;

initHMI_error:
	exitHMI();

initHMI_out:
	return result;
}

void __exit exitHMI() {
	printk(KERN_INFO MODULE_LABEL "Unloading module...\n");

	// Tear down LEDs device
	exitLEDs();

	// Tear down switches device
	exitSwitches();

	// Tear down buttons device
	if (isButtonsDeviceSetUp) {
		misc_deregister(&buttonsDevice);
	}

	int i;
	for (i = 0; i < ARRAY_SIZE(buttonGPIOs); i++) {
		free_irq(gpio_to_irq(buttonGPIOs[i].gpio), (void *)i);
	}

	gpio_free_array(buttonGPIOs, ARRAY_SIZE(buttonGPIOs));

	printk(KERN_INFO MODULE_LABEL "...done. (unloading)\n");
}

module_init(initHMI);
module_exit(exitHMI);
