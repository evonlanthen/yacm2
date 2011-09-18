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

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int powerState;
	MachineState machineState;
	unsigned int withMilk;
	Availability coffeeAvailability;
	Availability waterAvailability;
	Availability milkAvailability;
	unsigned int productIndex;
	unsigned int wasteBinFull;
MESSAGE_CONTENT_DEFINITION_END(Display, ChangeViewCommand)

MESSAGE_CONTENT_DEFINITION_BEGIN
	char message[128];
MESSAGE_CONTENT_DEFINITION_END(Display, ShowErrorCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(Display, Result)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(Display, ChangeViewCommand)
	MESSAGE_CONTENT(Display, ShowErrorCommand)
	MESSAGE_CONTENT(Display, Result)
MESSAGE_DEFINITION_END(Display)

#define CHANGE_VIEW_COMMAND 1

extern ActivityDescriptor getDisplayDescriptor(void);

#endif /* DISPLAY_H_ */
