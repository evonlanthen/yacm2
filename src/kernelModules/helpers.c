/**
 * @brief	Some helpers
 * @file    helpers.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#include "helpers.h"

#include <linux/kernel.h>
#include <linux/string.h>

unsigned long toInteger(const char *buffer) {
	return simple_strtoul(buffer, NULL, 0);
}

size_t toString(char *buffer, size_t size, unsigned char value) {
	memset(buffer, 0, size);
	sprintf(buffer, "%u", value);
	return strlen(buffer);
}
