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

MESSAGE_CONTENT_DEFINITION_BEGIN
	Byte code;
	char message[128];
MESSAGE_CONTENT_DEFINITION_END(UserInterface, Status)

MESSAGE_CONTENT_DEFINITION_BEGIN
	Byte command;
MESSAGE_CONTENT_DEFINITION_END(UserInterface, Command)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int viewType;
MESSAGE_CONTENT_DEFINITION_END(UserInterface, ChangeViewCommand)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(UserInterface, Command)
	MESSAGE_CONTENT(UserInterface, ChangeViewCommand)
	MESSAGE_CONTENT(UserInterface, Status)
MESSAGE_DEFINITION_END(UserInterface)

#define CHANGE_VIEW_COMMAND 1

extern ActivityDescriptor getUserInterfaceDescriptor(void);

#endif /* USERINTERFACE_H_ */
