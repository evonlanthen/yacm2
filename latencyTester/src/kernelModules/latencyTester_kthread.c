/**
 * @brief	Latency tester device
 * @file    latencyTester.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 30, 2011
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <rtdm_driver.h>
#include <mach/pxa2xx-regs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Toni Baumann, Ronny Stauffer, Elmar Vonlanthen");
MODULE_DESCRIPTION("Latency tester kthread");

#define MODULE_LABEL "LatencyTesterKthread"

//#define TEST_ISR
#define TEST_KERNEL_THREAD
//#define TEST_USER_THREAD

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

#define RTDM_TASK_HIGHEST_PRIORITY XNSCHED_HIGH_PRIO

/**
 *******************************************************************************
 * Global variables, structures and declarations
 *******************************************************************************
 */

// latencyTester GPIOS
static struct gpio latencyTesterGpios[] = {
	{ GPIO_INDEX, GPIOF_IN, "INDEX" },					/* INDEX (GPIO 35): Input */
	{ GPIO_TRIGGER, GPIOF_OUT_INIT_LOW, "TRIGGER" },	/* TRIGGER (GPIO 16): Output, default low */
	{ GPIO_SSPSCLK, GPIOF_OUT_INIT_LOW, "SSPSCLK" },	/* SSPSCLK (GPIO 23): Alternate Function 2 Output */
	{ GPIO_SSPSFRM, GPIOF_OUT_INIT_LOW, "SSPSFRM" },	/* SSPSFRM (GPIO 24): Alternate Function 2 Output */
	{ GPIO_SSPTXD, GPIOF_OUT_INIT_LOW, "SSPTXD" },		/* SSPTxD (GPIO 25): Alternate Function 2 Output */
	{ GPIO_SSPRXD, GPIOF_IN, "SSPRXD" },				/* SSPRxD (GPIO 26): Alternate Function 1 Input */
	{ GPIO_SPICS, GPIOF_OUT_INIT_HIGH, "SPICS" },		/* SPI CS (GPIO 22): Output, default high */
};

static int open(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, int oflag);
static ssize_t read(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, void *buffer, size_t size);
static ssize_t write(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, const void *buffer, size_t size);
static int close(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo);

static rtdm_mutex_t mutex;
static rtdm_event_t interruptEvent;
static rtdm_irq_t interrupt;
#ifdef TEST_KERNEL_THREAD
	static rtdm_task_t latencyTesterTask;
	static int isTaskSetUp = 0;
#endif
static int isDeviceSetUp = 0;
static int isInterruptSetUp = 0;

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

#ifdef TEST_KERNEL_THREAD
void taskProcedure() {
	while(1) {
		rtdm_event_wait(&interruptEvent);
		gpio_set_value(GPIO_TRIGGER, HIGH);
		gpio_set_value(GPIO_TRIGGER, LOW);
	}
}
#endif

static int interruptHandler(rtdm_irq_t *interrupt) {
#ifdef TEST_ISR
	gpio_set_value(GPIO_TRIGGER, HIGH);
	gpio_set_value(GPIO_TRIGGER, LOW);
#elif defined(TEST_KERNEL_THREAD)
	rtdm_event_signal(&interruptEvent);
#elif defined(TEST_USER_THREAD)
	rtdm_event_signal(&interruptEvent);
#endif
	return RTDM_IRQ_HANDLED;
}

static int open(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, int oflag) {
	return 0;
}

static ssize_t read(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, void *buffer, size_t size) {
	ssize_t result = 0;

#ifdef TEST_USER_THREAD
	rtdm_event_wait(&interruptEvent);
#endif

	return result;
}

static ssize_t write(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo, const void *buffer, size_t size) {
	ssize_t result = 0;

	gpio_set_value(GPIO_TRIGGER, HIGH);
	gpio_set_value(GPIO_TRIGGER, LOW);

	return result;
}

static int close(struct rtdm_dev_context *context, rtdm_user_info_t *userInfo) {
	return 0;
}

static struct rtdm_device latencyTesterDevice = {
	.struct_version = RTDM_DEVICE_STRUCT_VER,
	.device_flags = RTDM_NAMED_DEVICE,
	.context_size = 0,
	.device_name = "latencyTester",
	.open_nrt = open,
	.ops = {
		.read_rt = read,
		.write_rt = write,
		.close_nrt = close
	},
	.device_class = RTDM_CLASS_EXPERIMENTAL,
	.device_sub_class = RTDM_SUBCLASS_GENERIC,
	.profile_version = 1,
	.driver_name = "LatencyTester",
	.driver_version = RTDM_DRIVER_VER(0,1,0),
	.peripheral_name = "Latency tester",
	.provider_name = "Toni Baumann, Ronny Stauffer, Elmar Vonlanthen",
	.proc_name = latencyTesterDevice.device_name
};

static void __exit exitLatencyTester(void);

static int __init initLatencyTester(void) {
	int result = 0;

	rtdm_printk(KERN_INFO MODULE_LABEL "Loading module (by process \"%s\" (PID: %i)...\n", current->comm, current->pid);

	// Initialize mutex
  	rtdm_mutex_init(&mutex);

	// Claim GPIOs
	if ((result = gpio_request_array(latencyTesterGpios, ARRAY_SIZE(latencyTesterGpios)))) {
		rtdm_printk(KERN_WARNING MODULE_LABEL "Error requesting latencyTester GPIO array!\n");

	    goto initLatencyTester_error;
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

	int irqNumber = gpio_to_irq(GPIO_INDEX);
	rtdm_printk("Interrupt number: %d\n", irqNumber);

	if ((result = request_irq(irqNumber,
			interruptHandlerDummy,
			IRQF_TRIGGER_RISING,
			"INDEX",
			(void *)0))) {
		goto initLatencyTester_error;
	}
	free_irq(irqNumber, (void *)0);

	if ((result = rtdm_irq_request(&interrupt,
			irqNumber,
			interruptHandler,
			0,
			"INDEX",
			(void *)0))) {
		rtdm_printk(KERN_WARNING MODULE_LABEL "Error requesting latencyTester index GPIO interrupt!\n");

		goto initLatencyTester_error;
	}
	isInterruptSetUp = 1;

#ifdef TEST_KERNEL_THREAD
	if (rtdm_task_init(&latencyTesterTask, "latencyTesterTask", taskProcedure, NULL, RTDM_TASK_HIGHEST_PRIORITY, 0)) {
		printk(KERN_WARNING MODULE_LABEL "Error initializing latencyTester task!\n");
		goto initLatencyTester_error;
	}
	isTaskSetUp = 1;
#endif

	if ((result = rtdm_dev_register(&latencyTesterDevice))) {
		printk(KERN_WARNING MODULE_LABEL "Error registering latencyTester device!\n");

		goto initLatencyTester_error;
	}
	isDeviceSetUp = 1;

	rtdm_printk(KERN_INFO MODULE_LABEL "...done. (loading)\n");

	goto initLatencyTester_out;

initLatencyTester_error:
	exitLatencyTester();

initLatencyTester_out:
	return result;
}

static void __exit exitLatencyTester(void) {
	rtdm_printk(KERN_INFO MODULE_LABEL "Unloading module (by process \"%s\" (PID: %i)...\n", current->comm, current->pid);

	if (isDeviceSetUp) {
		rtdm_dev_unregister(&latencyTesterDevice, 0);
	}

#ifdef TEST_KERNEL_THREAD
	if (isTaskSetUp) {
		rtdm_task_destroy(&latencyTesterTask);
	}
#endif

	if (isInterruptSetUp) {
		//free_irq(gpio_to_irq(GPIO_INDEX), (void *)0);
		rtdm_irq_free(&interrupt);
	}

	rtdm_event_destroy(&interruptEvent);

	gpio_free_array(latencyTesterGpios, ARRAY_SIZE(latencyTesterGpios));

	rtdm_printk(KERN_INFO MODULE_LABEL "...done. (unloading)\n");
}

module_init(initLatencyTester);
module_exit(exitLatencyTester);
