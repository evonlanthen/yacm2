/**
 * @brief	grinder controlling device
 * @file    grinder.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <mach/pxa2xx-regs.h>
//#include <mach/regs-ssp.h> // not available anymore

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Toni Baumann, Ronny Stauffer, Elmar Vonlanthen");
MODULE_DESCRIPTION("Grinder controlling device");

/**
 *******************************************************************************
 * Defines
 *******************************************************************************
 */

//#define TASKLET
//#define KTHREAD

/* No header files available for PWM registers: */
#define PWMCR3		__REG(0x40C00010)	/* PWM control register 3 */
#define PWMDCR3		__REG(0x40C00014)	/* PWM duty cycle register 3 [0-99] */
#define PWMPCR3		__REG(0x40C00018)	/* PWM period control register 3 */

/* No header files available for SPI registers: */
#define SSCR0_P1	__REG(0x41000000)	/* PWM control register 3 */
#define SSCR1_P1	__REG(0x41000004)	/* PWM duty cycle register 3 */
#define SSDR_P1		__REG(0x41000010)	/* PWM period control register 3 */
#define SSCR1_SPO			(1 << 3)	/* Motorola SPI SSPSCLK polarity setting */
#define SSCR1_SPH			(1 << 4)	/* Motorola SPI SSPSCLK phase setting */
#define SSCR0_SSE			(1 << 7)	/* Synchronous Serial Port Enable */
#define SSCR0_DataSize(x)	((x) - 1)	/* Data Size Select [4..16] */

/* define alternate function gpio values: */
#define GPIO_ALT_FN_1_IN	0x100
#define GPIO_ALT_FN_1_OUT	0x180
#define GPIO_ALT_FN_2_IN	0x200
#define GPIO_ALT_FN_2_OUT	0x280
#define GPIO_ALT_FN_3_IN	0x300
#define GPIO_ALT_FN_3_OUT	0x380

/* define gpio numbers: */
#define GPIO_PWM3			12
#define GPIO_DIR			21
#define GPIO_INDEX			35
#define GPIO_TRIGGER		16
#define GPIO_SSPSCLK		23
#define GPIO_SSPSFRM		24
#define GPIO_SSPTXD			25
#define GPIO_SSPRXD			26
#define GPIO_SPICS			22

/* define high and low: */
#define HIGH				1
#define LOW					0

/**
 *******************************************************************************
 * Global variables, structures and declarations
 *******************************************************************************
 */

// grinder gpios:
static struct gpio gpioGrinder[] = {
	{ GPIO_PWM3, GPIOF_OUT_INIT_LOW, "PWM3" },			/* PWM3 (GPIO 12, velocity): Alternate Function 2 Output */
	{ GPIO_DIR, GPIOF_OUT_INIT_LOW, "DIR" },			/* DIR (GPIO 21): Output, default low */
	{ GPIO_INDEX, GPIOF_IN, "INDEX" },					/* INDEX (GPIO 35): Input */
	{ GPIO_TRIGGER, GPIOF_OUT_INIT_LOW, "TRIGGER" },	/* TRIGGER (GPIO 16): Output, default low */
	{ GPIO_SSPSCLK, GPIOF_OUT_INIT_LOW, "SSPSCLK" },	/* SSPSCLK (GPIO 23): Alternate Function 2 Output */
	{ GPIO_SSPSFRM, GPIOF_OUT_INIT_LOW, "SSPSFRM" },	/* SSPSFRM (GPIO 24): Alternate Function 2 Output */
	{ GPIO_SSPTXD, GPIOF_OUT_INIT_LOW, "SSPTXD" },		/* SSPTxD (GPIO 25): Alternate Function 2 Output */
	{ GPIO_SSPRXD, GPIOF_IN, "SSPRXD" },				/* SSPRxD (GPIO 26): Alternate Function 1 Input */
	{ GPIO_SPICS, GPIOF_OUT_INIT_HIGH, "SPICS" },		/* SPI CS (GPIO 22): Output, default high */
};

int grinderOpen (struct inode *inode, struct file *file);
int grinderRelease(struct inode *inode, struct file *file);
ssize_t grinderWrite(struct file *filePointer, const char __user *buffer, size_t length, loff_t *offset);

static struct file_operations grinderFileOperations = {
	.owner   = THIS_MODULE,
	.open    = grinderOpen,
	.write   = grinderWrite,
	.release = grinderRelease,
};

static struct miscdevice grinderDevice = {
	.minor   = MISC_DYNAMIC_MINOR,
	.name    = "coffeeGrinderMotor",
	.fops    = &grinderFileOperations,
	.mode    = 00666
};

static struct miscdevice grinderSPIDevice = {
	.minor   = MISC_DYNAMIC_MINOR,
	.name    = "coffeeGrinderSPI",
	.fops    = &grinderFileOperations,
	.mode    = 00666
};

#ifdef TASKLET
static void taskletHandler(unsigned long data);
static struct tasklet_struct grinderTasklet = {
	.func = taskletHandler,
	.data = 0
};
#endif

struct mutex mutex;

/**
 *******************************************************************************
 * Helper functions
 *******************************************************************************
 */

void __init gpio_fn_clear(int gpio) {
	unsigned long flags;
	int gafr;

	int fn = (gpio & 0x300) >> 8;
	local_irq_save(flags);
	gafr = GAFR(gpio) & ~(0x3 << (((gpio) & 0xf)*2));
	GAFR(gpio) = gafr |  (fn  << (((gpio) & 0xf)*2));
	local_irq_restore(flags);
}

void __init gpio_fn_set(int gpio, int mode) {
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

static irqreturn_t interruptHandler(int irq, void *deviceId) {
	//printk(KERN_INFO "grinder interrupt handler\n");
#ifdef KTHREAD
	return IRQ_WAKE_THREAD;
#elif defined(TASKLET)
	tasklet_schedule(&grinderTasklet);
	return IRQ_HANDLED;
#else
	gpio_set_value(GPIO_TRIGGER, HIGH);
	gpio_set_value(GPIO_TRIGGER, LOW);
	return IRQ_HANDLED;
#endif
}

#ifdef KTHREAD
static irqreturn_t interruptThread(int irq, void *dev_id) {
	//printk(KERN_INFO "grinder thread handler\n");
	gpio_set_value(GPIO_TRIGGER, HIGH);
	gpio_set_value(GPIO_TRIGGER, LOW);
	return IRQ_HANDLED;
}
#elif defined(TASKLET)
static void taskletHandler(unsigned long data) {
	//printk(KERN_INFO "grinder tasklet handler\n");
	gpio_set_value(GPIO_TRIGGER, HIGH);
	gpio_set_value(GPIO_TRIGGER, LOW);
}
#endif

ssize_t grinderWrite(struct file *filePointer, const char __user *buffer, size_t length, loff_t *offset) {
	ssize_t result = 0;
	char *data;
	int value;

	data = kzalloc(length+1, GFP_KERNEL);
	if (!(data)) {
		printk(KERN_WARNING "grinderWrite: kzalloc failed!\n");
			result = -ENOMEM;
			goto out;
	}

	if (copy_from_user(data, buffer, length)) {
		printk(KERN_WARNING "grinderWrite: copy_from_user failed!\n");
		result = -EFAULT;
		goto out;
	}
	value = simple_strtol(data, NULL, 0);
	kfree(data);

	/* enter critical section: */
	if (mutex_lock_interruptible(&mutex)) {
		result = -ERESTARTSYS;
		goto out;
	}

	if (strcmp(filePointer->f_path.dentry->d_name.name, "coffeeGrinderSPI") == 0) {
		/* set flash duration: */
		if (value >= 0 && value < 256) {
			printk(KERN_INFO "grinderWrite: setting spi value to %d\n", value);
			SSDR_P1 = value;
		} else {
			printk(KERN_WARNING "grinderWrite: spi value %d is invalid\n", value);
		}
	} else {
		/* set speed: */
		if (value >= 0 && value < 100) {
			printk(KERN_INFO "grinderWrite: setting pwm3 value to %d\n", value);
			PWMDCR3 = value;
		} else {
			printk(KERN_WARNING "grinderWrite: pwm3 value %d is invalid\n", value);
		}
	}

	/* leave critical section: */
	mutex_unlock(&mutex);

	result = length;

	out:
	  return result;
}

int grinderOpen (struct inode *inode, struct file *file) {
	printk(KERN_INFO "grinderOpen: process is \"%s\" (pid: %i)\n", current->comm, current->pid);
	return 0;
}

int grinderRelease(struct inode *inode, struct file *file) {
	printk(KERN_INFO "grinderRelease: process is \"%s\" (pid: %i)\n", current->comm, current->pid);
	return 0;
}

int __init grinderInit(void) {
	int result = 0, error;
	unsigned long flags = 0;

	/* initialize mutex: */
  	mutex_init(&mutex);

	/* request gpios: */
	error = gpio_request_array(gpioGrinder, ARRAY_SIZE(gpioGrinder));
	if (error) {
		printk(KERN_WARNING "grinderInit: error requesting gpio array!\n");
	    result = error;
	    goto err_gpio;
	}

	/* set alternate functions: */
	gpio_fn_set(GPIO_PWM3, GPIO_ALT_FN_2_OUT);
	gpio_fn_set(GPIO_TRIGGER, 0);
	gpio_fn_set(GPIO_SSPSCLK, GPIO_ALT_FN_2_OUT);
	gpio_fn_set(GPIO_SSPSFRM, GPIO_ALT_FN_2_OUT);
	gpio_fn_set(GPIO_SSPTXD, GPIO_ALT_FN_2_OUT);
	gpio_fn_set(GPIO_SSPRXD, GPIO_ALT_FN_1_IN);

	/* prevent race conditions with interrupts: */
	local_irq_save(flags);

	printk(KERN_INFO "grinderInit: process is \"%s\" (pid: %i)\n", current->comm, current->pid);
	/* set prescale value, periodic interval and speed for PWM3: */
	PWMCR3 = 0x14;						/* divide 13MHz clock by 0x14 */
	PWMDCR3 = 0;						/* initial speed is zero */
	PWMPCR3 = 100;						/* periode is 100 time ticks */

	/* enable clock for PWM3: */
	CKEN |= 0x03;

	/* set stroboscope registers: */
	SSCR0_P1 &= ~SSCR0_SSE;				/* disable synchronous serial */
	SSCR0_P1 = SSCR0_DataSize(8);		/* data size is 8 bit */
	SSCR1_P1 = SSCR1_SPH | SSCR1_SPO;	/* Motorola SPI */
	CKEN |= (1 << CKEN_SSP1);			/* enable SSP1 Unit Clock */
	SSCR0_P1 |= SSCR0_SSE;				/* enable synchronous serial */
	SSDR_P1 = 50;						/* data register for flash time */
	  
	/* leave critical section: */
	local_irq_restore(flags);

#ifdef KTHREAD
    error = request_threaded_irq(gpio_to_irq(GPIO_INDEX),
    		interruptHandler,
    		interruptThread,
    		IRQF_TRIGGER_RISING,
    		"INDEX",
			(void *)0);
#else
	error = request_irq(gpio_to_irq(GPIO_INDEX),
			interruptHandler,
			IRQF_TRIGGER_RISING,
			"INDEX",
			(void *)0);
#endif
	if (error) {
		result = error;
		goto err_gpio;
	}

#ifdef TASKLET
	tasklet_init(&grinderTasklet, taskletHandler, 0);
#endif
	
	/* craete device /dev/coffeeGrinderMotor: */
	if ((result = misc_register(&grinderDevice))) {
		printk(KERN_WARNING "grinderInit: error registering grinder motor device!\n");
		goto err_gpio;
	}

	/* craete device /dev/coffeeGrinderSPI: */
	if ((result = misc_register(&grinderSPIDevice))) {
		printk(KERN_WARNING "grinderInit: error registering grinder SPI device!\n");
		goto err_spi;
	}

	printk(KERN_INFO "grinderInit: device successfully registered!\n");
	goto out;

	err_spi:
	  misc_deregister(&grinderDevice);

	err_gpio:
	  gpio_free_array(gpioGrinder, ARRAY_SIZE(gpioGrinder));

	out:
	  return result;
}

void __exit grinderExit(void) {
	misc_deregister(&grinderSPIDevice);
	misc_deregister(&grinderDevice);
#ifdef TASKLET
	tasklet_kill(&grinderTasklet);
#endif
	free_irq(gpio_to_irq(GPIO_INDEX), (void *)0);
	gpio_free_array(gpioGrinder, ARRAY_SIZE(gpioGrinder));
	printk(KERN_INFO "grinderExit: device unregistered!\n");
}

module_init(grinderInit);
module_exit(grinderExit);
