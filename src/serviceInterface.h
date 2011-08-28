/**
 * @brief   <description>
 * @file    serviceInterface.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 27, 2011
 */

#ifndef SERVICEINTERFACE_H_
#define SERVICEINTERFACE_H_

#include <mqueue.h>
#include "activity.h"

typedef struct {
	int intValue;
	char strValue[256];
} ServiceInterfaceMessage;

extern ActivityDescriptor getServiceInterfaceDescriptor(void);

#endif /* SERVICEINTERFACE_H_ */
