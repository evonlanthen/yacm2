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

typedef enum {
	ss_coffeeSupply = 0,
	ss_milkSupply,
	ss_waterSupply,
	ss_serviceInterface,
	ss_userInterface
} SubSystem;

typedef struct {
	SubSystem subSystem;
	int intValue;
	char strValue[256];
} MainControllerMessage;

extern ActivityDescriptor getMainControllerDescriptor(void);

#endif /* MAINCONTROLLER_H_ */
