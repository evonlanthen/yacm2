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

#include <linux/wait.h>

#include <linux/bitops.h>

#include "switches.h"
#include "leds.h"
#include "hmi.h"

static struct timer_list debounceButtonTimers[4];
static volatile unsigned long debounceButtonFlags = 0;

static wait_queue_head_t isDataAvailableWaitQueue;
static volatile int lastPressedButtonIndex = 0;

static volatile unsigned long bit = 0;
static spinlock_t spinlock = SPIN_LOCK_UNLOCKED;

static ssize_t read(struct file *file, char __user *buffer, size_t size, loff_t *offset) {
	ssize_t result;

	if (size < 1) {
		return -EFAULT;
	}

	if (*offset == 0) {
		wait_event_interruptible(isDataAvailableWaitQueue, test_and_clear_bit(0, &bit));

		// Open atomic context (by disabling interrupts)
		unsigned long flags;
		spin_lock_irqsave(&spinlock, flags);

		unsigned char value = 0x30 + lastPressedButtonIndex;

		// Close atomic context (by enabling interrupts)
		spin_unlock_irqrestore(&spinlock, flags);

		if (copy_to_user(buffer, &value, 1)) {
			result = -EFAULT;
		} else {
			*offset = 1;
			result = 1;
		}
	} else {
		result = /* EOF: */ 0;
	}

	return result;
}

static struct file_operations buttonsFileOperations = {
	.owner = THIS_MODULE,
	.read = read,
	.llseek = no_llseek
};

static struct miscdevice buttonsDevice = {
	.name = "buttons",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &buttonsFileOperations,
};

static struct gpio buttonGPIOs[] = {
	{ 99, GPIOF_IN, "T0" },
	{ 101, GPIOF_IN, "T1" },
	{ 102, GPIOF_IN, "T2" },
	{ 103, GPIOF_IN, "T3" }
};

static irqreturn_t handleButtonInterrupt(int irq, void *dev_id) {
	int index = (int)dev_id;

	printk(KERN_INFO "Interrupt for button %d (button GPIO %d) occured!\n", index, buttonGPIOs[index].gpio);

	if (test_and_set_bit(index, &debounceButtonFlags)) {
		printk(KERN_INFO "Ignoring interrupt for GPIO %d.\n", buttonGPIOs[index].gpio);
		return IRQ_HANDLED;
	}

	lastPressedButtonIndex = index;
	set_bit(0, &bit);
	wake_up_interruptible_all(&isDataAvailableWaitQueue);

	mod_timer(&debounceButtonTimers[index], jiffies + msecs_to_jiffies(100));

	return IRQ_WAKE_THREAD;
}

static irqreturn_t handleButtonInterrupt2ndStage(int irq, void *dev_id) {
	int index = (int)dev_id;
	//int led_gpio = gpio_leds[index+4].gpio;

	//gpio_set_value(led_gpio, !gpio_get_value(led_gpio));

	return IRQ_HANDLED;
}

static void buttonDebounceTimerExpired(unsigned long data) {
	printk(KERN_INFO "Debounce timer for button GPIO %d expired.\n", buttonGPIOs[data].gpio);
	clear_bit(data, &debounceButtonFlags);
}

static int isButtonsDeviceSetUp = 0;

void __exit exitHMI(void);
int __init initHMI() {
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

		if ((result = request_threaded_irq(buttonGPIOInterrupt,
								handleButtonInterrupt,
								handleButtonInterrupt2ndStage,
								IRQF_TRIGGER_RISING,
								buttonGPIOs[i].label,
								(void *)i))) {
			printk(KERN_WARNING MODULE_LABEL "Error requesting buttons GPIO interrupts!\n");

			goto initHMI_error;
		}
	}

	// 1.5 Initialize wait queue (used for signaling to waiting read system calls that data is available)
	init_waitqueue_head(&isDataAvailableWaitQueue);

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
	//gpio_free_array(enableGPIOs, ARRAY_SIZE(enableGPIOs));

	printk(KERN_INFO MODULE_LABEL "...done. (unloading)\n");
}

module_init(initHMI);
module_exit(exitHMI);
