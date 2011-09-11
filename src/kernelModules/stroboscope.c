/**
 * stroboscope controller
 *
 * @file    stroboscope.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>

//#include <mach/gpio.h>
//#include <mach/colibri.h>
//#include <mach/pxa27x.h>
//#include <mach/mfp-pxa27x.h>
//#include <linux/ioport.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Toni Baumann, Ronny Stauffer, Elmar Vonlanthen");
MODULE_DESCRIPTION("Stroboscope controller");

// TODO: remove workaround (where is this defined?):
#define GPIO_ALT_FN_1_IN 0x100
#define GPIO_ALT_FN_1_OUT 0x180
#define GPIO_ALT_FN_2_IN 0x200
#define GPIO_ALT_FN_2_OUT 0x280

// stroboscope gpios:
static struct gpio gpioStroboscope[] = {
	{ 23, GPIOF_OUT_INIT_LOW | GPIO_ALT_FN_2_OUT, "SSPSCLK" },	// SSPSCLK (GPIO 23): Alternate Function 2 Output
	{ 24, GPIOF_OUT_INIT_LOW | GPIO_ALT_FN_2_OUT, "SSPSFRM" },	// SSPSFRM (GPIO 24): Alternate Function 2 Output
	{ 25, GPIOF_OUT_INIT_LOW | GPIO_ALT_FN_2_OUT, "SSPTXD" },	// SSPTxD (GPIO 25): Alternate Function 2 Output
	{ 26, GPIOF_IN | GPIO_ALT_FN_1_OUT, "SSPRXD" },				// SSPRxD (GPIO 26): Alternate Function 1 Input
	{ 22, GPIOF_OUT_INIT_HIGH, "SPICS" },						// SPI CS (GPIO 22): Output, default high
};

// gpio helper function:
void gpio_mode_clear(int gpio) {
  unsigned long flags;
  int gafr;

  int fn = (gpio & 0x300) >> 8;
  local_irq_save(flags);
  gafr = GAFR(gpio) & ~(0x3 << (((gpio) & 0xf)*2));
  GAFR(gpio) = gafr |  (fn  << (((gpio) & 0xf)*2));
  local_irq_restore(flags);
}

int __init stroboscopeInit(void) {
	int result = 0, error;
	unsigned long flags;

	// prevent race conditions:
	local_irq_save(flags);

	// request gpios:
	error = gpio_request_array(gpioStroboscope, ARRAY_SIZE(gpioStroboscope));
	if (error) {
	    result = error;
	    goto err;
	}

	// TODO: set alternate function 2:
	// SSPSCLK (GPIO 23): Alternate Function 2 Output
	// SSPSFRM (GPIO 24): Alternate Function 2 Output
	// SSPTxD (GPIO 25): Alternate Function 2 Output
	// SSPRxD (GPIO 26): Alternate Function 1 Input

	// set registers:
	// TODO: where is this defined?
	//SSCR0_P1 &= ~SSCR0_SSE;				// disable synchronous serial
	//SSCR0_P1 = SSCR0_DataSize(8);		// data size is 8 bit
	//SSCR1_P1 = SSCR1_SPH | SSCR1_SPO;	// Motorola SPI
	//CKEN |= (1 << CKEN_SSP1);			// enable SSP1 Unit Clock
	//SSCR0_P1 |= SSCR0_SSE;				// enable synchronous serial

	printk(KERN_INFO "stroboscope device: successfully registered!\n");
	goto out;

	err:
	  gpio_free_array(gpioStroboscope, ARRAY_SIZE(gpioStroboscope));

	out:
	  local_irq_restore(flags);
	  return result;
}

void __exit stroboscopeExit(void) {
	gpio_free_array(gpioStroboscope, ARRAY_SIZE(gpioStroboscope));
	printk(KERN_INFO "stroboscope device: unregistered!\n");
}

module_init(stroboscopeInit);
module_exit(stroboscopeExit);

