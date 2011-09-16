/**
 * @brief   Some helpers
 * @file    helpers.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#ifndef HELPERS_H_
#define HELPERS_H_

#include <linux/types.h>

unsigned long toInteger(const char *buffer);
size_t toString(char *buffer, size_t size, unsigned char value);

#endif /* HELPERS_H_ */
