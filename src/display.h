/**
 * @brief   LED and graphical display
 * @file    display.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 29, 2011
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <mqueue.h>
#include "activity.h"

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} DisplayMessage;

extern ActivityDescriptor getDisplayDescriptor(void);

#endif /* DISPLAY_H_ */
