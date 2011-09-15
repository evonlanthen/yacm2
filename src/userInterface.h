/**
 * @brief   User interface
 * @file    userInterface.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef USERINTERFACE_H_
#define USERINTERFACE_H_

#include <mqueue.h>
#include "activity.h"

MESSAGE_CONTENT_DEFINITION_BEGIN(UserInterfaceStatus)
	Byte code;
	char message[128];
MESSAGE_CONTENT_DEFINITION_END

MESSAGE_CONTENT_DEFINITION_BEGIN(UserInterfaceCommand)
	Byte command;
MESSAGE_CONTENT_DEFINITION_END

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(UserInterfaceCommand)
	MESSAGE_CONTENT(UserInterfaceStatus)
MESSAGE_DEFINITION_END(UserInterface)

extern ActivityDescriptor getUserInterfaceDescriptor(void);

#endif /* USERINTERFACE_H_ */
