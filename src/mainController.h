/**
 * @brief   Main system control module
 * @file    mainController.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef MAINCONTROLLER_H_
#define MAINCONTROLLER_H_

#include <mqueue.h>
#include "activity.h"

#define WASTE_BIN_FULL_ERROR 1401
#define WASTE_BIN_CLEAR_NOTIFICATION 1301

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} MainControllerMessage;

extern ActivityDescriptor getMainControllerDescriptor(void);

#endif /* MAINCONTROLLER_H_ */
