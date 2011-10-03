/**
 * @brief	Display device
 * @file    display.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 26, 2011
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <rtdm_driver.h>

//#include <linux/slab.h>

//#include <linux/fs.h>

//#include <linux/uaccess.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>

//#include <linux/mutex.h>

#include <mach/pxa2xx-regs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Toni Baumann, Ronny Stauffer, Elmar Vonlanthen");
MODULE_DESCRIPTION("RT-Model display");

#define MODULE_LABEL "rt-model-display "

/**
 *******************************************************************************
 * Defines
 *******************************************************************************
 */

// SPI registers
#define SSCR0_P1	__REG(0x41000000)	/* SPI control register 0 */
#define SSCR1_P1	__REG(0x41000004)	/* SPI control register 1 */
#define SSDR_P1		__REG(0x41000010)	/* SPI data register */
#define SSCR1_SPO			(1 << 3)	/* Motorola SPI SSPSCLK polarity setting */
#define SSCR1_SPH			(1 << 4)	/* Motorola SPI SSPSCLK phase setting */
#define SSCR0_SSE			(1 << 7)	/* Synchronous Serial Port Enable */
#define SSCR0_DataSize(x)	((x) - 1)	/* Data Size Select [4..16] */

// GPIO alternate function values
#define GPIO_ALT_FN_1_IN	0x100
#define GPIO_ALT_FN_1_OUT	0x180
#define GPIO_ALT_FN_2_IN	0x200
#define GPIO_ALT_FN_2_OUT	0x280
#define GPIO_ALT_FN_3_IN	0x300
#define GPIO_ALT_FN_3_OUT	0x380

// GPIO numbers
#define GPIO_INDEX			35
#define GPIO_TRIGGER		16
#define GPIO_SSPSCLK		23
#define GPIO_SSPSFRM		24
#define GPIO_SSPTXD			25
#define GPIO_SSPRXD			26
#define GPIO_SPICS			22

// GPIO levels
#define HIGH				1
#define LOW					0

/**
 *******************************************************************************
 * Global variables, structures and declarations
 *******************************************************************************
 */

// Display GPIOS
static struct gpio displayGpios[] = {
	{ GPIO_INDEX, GPIOF_IN, "INDEX" },					/* INDEX (GPIO 35): Input */
	{ GPIO_TRIGGER, GPIOF_OUT_INIT_LOW, "TRIGGER" },	/* TRIGGER (GPIO 16): Output, default low */
	{ GPIO_SSPSCLK, GPIOF_OUT_INIT_LOW, "SSPSCLK" },	/* SSPSCLK (GPIO 23): Alternate Function 2 Output */
	{ GPIO_SSPSFRM, GPIOF_OUT_INIT_LOW, "SSPSFRM" },	/* SSPSFRM (GPIO 24): Alternate Function 2 Output */
	{ GPIO_SSPTXD, GPIOF_OUT_INIT_LOW, "SSPTXD" },		/* SSPTxD (GPIO 25): Alternate Function 2 Output */
	{ GPIO_SSPRXD, GPIOF_IN, "SSPRXD" },				/* SSPRxD (GPIO 26): Alternate Function 1 Input */
	{ GPIO_SPICS, GPIOF_OUT_INIT_HIGH, "SPICS" },		/* SPI CS (GPIO 22): Output, default high */
};

//static int open(struct inode *inode, struct file *file);
static int open(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, int oflag);
static ssize_t read(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, void *buffer, size_t size);
//static ssize_t write(struct file *file, const char __user *buffer, size_t length, loff_t *offset);
static ssize_t write(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, const void *buffer, size_t size);
//static int close(struct inode *inode, struct file *file);
static int close(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo);

static rtdm_mutex_t mutex;

/**
 *******************************************************************************
 * Helper functions
 *******************************************************************************
 */

static void __init clearGpioFunction(int gpio) {
	unsigned long flags;
	int gafr;

	int fn = (gpio & 0x300) >> 8;
	local_irq_save(flags);
	gafr = GAFR(gpio) & ~(0x3 << (((gpio) & 0xf)*2));
	GAFR(gpio) = gafr |  (fn  << (((gpio) & 0xf)*2));
	local_irq_restore(flags);
}

static void __init setGpioFunction(int gpio, int mode) {
	unsigned long flags;
	int gafr; int _gpio = gpio | mode;

	int fn = (_gpio & 0x300) >> 8;
	local_irq_save(flags);
	gafr = GAFR(_gpio) & ~(0x3 << (((_gpio) & 0xf)*2));
	GAFR(_gpio) = gafr |  (fn  << (((_gpio) & 0xf)*2));
	local_irq_restore(flags);
}

/**
 *******************************************************************************
 * Main functions
 *******************************************************************************
 */

static irqreturn_t interruptHandlerDummy(int irq, void *deviceId) {
	return IRQ_HANDLED;
}

static rtdm_event_t interruptEvent;

static int interruptHandler(rtdm_irq_t *interrupt) {
	//rtdm_printk("Interrupt occured!\n");

	//gpio_set_value(GPIO_TRIGGER, HIGH);
	//gpio_set_value(GPIO_TRIGGER, LOW);

	rtdm_event_signal(&interruptEvent);

	return RTDM_IRQ_HANDLED;
}

//static int open(struct inode *inode, struct file *file) {
static int open(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, int oflag) {
	//rtdm_printk(KERN_INFO MODULE_LABEL "Opening device (by process \"%s\" (PID: %i)...\n", current->comm, current->pid);

	return 0;
}

static ssize_t read(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, void *buffer, size_t size) {
	// Wait for interrupt...
	rtdm_event_wait(&interruptEvent);

	return 0;
}

//static ssize_t write(struct file *file, const char __user *buffer, size_t size, loff_t *offset) {
static ssize_t write(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, const void *buffer, size_t size) {
	ssize_t result = 0;

//	char *message;
//	if (!(message = rtdm_malloc(size + 1))) {
//		return -ENOMEM;
//	}

//	if (!copy_from_user(message, buffer, size)) {
//		// Critical section:
//		if (rtdm_mutex_lock(&mutex)) {
//			result = -ERESTARTSYS;
//
//			goto write_lockError;
//		}
//
//		//if (strcmp(file->f_path.dentry->d_name.name, "display") == 0) {
//			//SSDR_P1 = value;
//		//}
//
//		rtdm_mutex_unlock(&mutex);
//	} else {
//		printk(KERN_WARNING MODULE_LABEL "Error copying data to kernel memory!");
//		result = /* Error copying data! */ -EFAULT;
//
//		goto write_copyError;
//	}

	gpio_set_value(GPIO_TRIGGER, HIGH);
	gpio_set_value(GPIO_TRIGGER, LOW);

	goto write_out;

write_copyError:
write_lockError:
//	rtdm_free(message);

write_out:
	return result;
}

//static int close(struct inode *inode, struct file *file) {
static int close(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo) {
	//rtdm_printk(KERN_INFO MODULE_LABEL "Releasing device (by process \"%s\" (PID: %i)...\n", current->comm, current->pid);

	return 0;
}

static struct rtdm_device displayDevice = {
	.struct_version = RTDM_DEVICE_STRUCT_VER,
	.device_flags = RTDM_NAMED_DEVICE,
	.context_size = 0,
	.device_name = "rt-model-display",
	.open_nrt = open,
	.ops = {
		.read_rt = read,
		.write_rt = write,
		.close_nrt = close
	},
	.device_class = RTDM_CLASS_EXPERIMENTAL,
	.device_sub_class = RTDM_SUBCLASS_GENERIC,
	.profile_version = 1,
	.driver_name = "display",
	.driver_version = RTDM_DRIVER_VER(0,1,0),
	.peripheral_name = "RT-Model display",
	.provider_name = "Toni Baumann, Ronny Stauffer, Elmar Vonlanthen",
	.proc_name = displayDevice.device_name
};

static int isDisplayDeviceSetUp = 0;

static int isInterruptSetUp = 0;
static rtdm_irq_t interrupt;

static void __exit exitDisplay(void);

static int __init initDisplay(void) {
	int result = 0;

	rtdm_printk(KERN_INFO MODULE_LABEL "Loading module (by process \"%s\" (PID: %i)...\n", current->comm, current->pid);

	// Initialize mutex
  	rtdm_mutex_init(&mutex);

	// Claim GPIOs
	if ((result = gpio_request_array(displayGpios, ARRAY_SIZE(displayGpios)))) {
		rtdm_printk(KERN_WARNING MODULE_LABEL "Error requesting display GPIO array!\n");

	    goto initDisplay_error;
	}

	// Set alternate GPIO functions
	setGpioFunction(GPIO_TRIGGER, 0);
	setGpioFunction(GPIO_SSPSCLK, GPIO_ALT_FN_2_OUT);
	setGpioFunction(GPIO_SSPSFRM, GPIO_ALT_FN_2_OUT);
	setGpioFunction(GPIO_SSPTXD, GPIO_ALT_FN_2_OUT);
	setGpioFunction(GPIO_SSPRXD, GPIO_ALT_FN_1_IN);

	// Initialize stroboscope
	// Critical section:
	// Disable interrupts in order to prevent race conditions
	unsigned long flags = 0;
	rtdm_lock_irqsave(flags);

	SSCR0_P1 &= ~SSCR0_SSE;				/* Disable synchronous serial */
	SSCR0_P1 = SSCR0_DataSize(8);		/* Data size is 8 bit */
	SSCR1_P1 = SSCR1_SPH | SSCR1_SPO;	/* Motorola SPI */
	CKEN |= (1 << CKEN_SSP1);			/* Enable SSP1 Unit Clock */
	SSCR0_P1 |= SSCR0_SSE;				/* Enable synchronous serial */
	SSDR_P1 = 200;						/* Data register for flash time */
	  
	rtdm_lock_irqrestore(flags);

	rtdm_event_init(&interruptEvent, 0);

//	if ((result = request_irq(gpio_to_irq(GPIO_INDEX),
//			interruptHandler,
//			IRQF_TRIGGER_RISING,
//			"INDEX",
//			(void *)0))) {
	int irqNumber = gpio_to_irq(GPIO_INDEX);
	rtdm_printk("Interrupt number: %d\n", irqNumber);

	if ((result = request_irq(irqNumber,
			interruptHandlerDummy,
			IRQF_TRIGGER_RISING,
			"INDEX",
			(void *)0))) {
		goto initDisplay_error;
	}
	free_irq(irqNumber, (void *)0);

	if ((result = rtdm_irq_request(&interrupt,
			irqNumber,
			interruptHandler,
			0,
			"INDEX",
			(void *)0))) {
		rtdm_printk(KERN_WARNING MODULE_LABEL "Error requesting display index GPIO interrupt!\n");

		goto initDisplay_error;
	}
//	if ((result = rtdm_irq_enable(&interrupt))) {
//		rtdm_printk(KERN_WARNING MODULE_LABEL "Error enabling display index GPIO interrupt!\n");
//	}
	isInterruptSetUp = 1;
	
	if ((result = rtdm_dev_register(&displayDevice))) {
		printk(KERN_WARNING MODULE_LABEL "Error registering display device!\n");

		goto initDisplay_error;
	}
	isDisplayDeviceSetUp = 1;

	rtdm_printk(KERN_INFO MODULE_LABEL "...done. (loading)\n");

	goto initDisplay_out;

initDisplay_error:
	exitDisplay();

initDisplay_out:
	return result;
}

static void __exit exitDisplay(void) {
	rtdm_printk(KERN_INFO MODULE_LABEL "Unloading module (by process \"%s\" (PID: %i)...\n", current->comm, current->pid);

	if (isDisplayDeviceSetUp) {
		rtdm_dev_unregister(&displayDevice, 0);
	}

	if (isInterruptSetUp) {
		//free_irq(gpio_to_irq(GPIO_INDEX), (void *)0);
		rtdm_irq_free(&interrupt);
	}

	rtdm_event_destroy(&interruptEvent);

	gpio_free_array(displayGpios, ARRAY_SIZE(displayGpios));

	rtdm_printk(KERN_INFO MODULE_LABEL "...done. (unloading)\n");
}

module_init(initDisplay);
module_exit(exitDisplay);
