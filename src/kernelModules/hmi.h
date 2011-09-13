/**
 * @brief	HMI device
 * @file    hmi.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 11, 2011
 */

#ifndef HMI_H_
#define HMI_H_

#define MODULE_LABEL "hmi "

/* CARME internal memory-mapped IO registers base address  */
#define MMIO_BASE 0x0c003000
//#define MMIO_LEN 0x1000

#define MMIO_LEDS_OFFSET 0x0
#define MMIO_SWITCHES_OFFSET 0x200

#endif /* HMI_H_ */
