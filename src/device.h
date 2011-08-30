/**
 * @brief   Device helper functions
 * @file    device.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef DEVICE_H_
#define DEVICE_H_

typedef enum {
	wrm_replace = 0,
	wrm_append
} WriteMode;

extern int readBlockingDevice(char *deviceFifo);
extern int readNonBlockingDevice(char *deviceFile);
extern int writeNonBlockingDevice(char *deviceFile, char *str, WriteMode mode, int newLine);

#endif /* DEVICE_H_ */
