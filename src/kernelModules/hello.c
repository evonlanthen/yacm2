/**
 * @brief	Hello world kernel module
 * @file    hello.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 8, 2011
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Toni Baumann, Ronny Stauffer, Elmar Vonlanthen");
MODULE_DESCRIPTION("Hello world example");

int __init hello_init(void) {
	printk(KERN_INFO "Hello World!\n");
	return 0;
}

void __exit hello_exit(void) {
	printk(KERN_INFO "Bye, bye!\n");
}

module_init(hello_init);
module_exit(hello_exit);

