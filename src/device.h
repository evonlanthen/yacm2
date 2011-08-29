/**
 * @brief   Device helper functions
 * @file    device.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef DEVICE_H_
#define DEVICE_H_

extern int readBlockableDevice(char *deviceFifo);
extern int readNonBlockableDevice(char *deviceFile);

#endif /* DEVICE_H_ */
