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

#define COFFEE_INDEX 1
#define WATER_INDEX 2
#define MILK_INDEX 3

#define WASTE_BIN_FULL_ERROR 1401
#define WASTE_BIN_CLEAR_NOTIFICATION 1301

//typedef struct {
//	ActivityDescriptor activity;
//	int intValue;
//	char strValue[256];
//} MainControllerMessage;

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, InitCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, OffCommand)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int productIndex;
	int withMilk;
MESSAGE_CONTENT_DEFINITION_END(MainController, ProduceProductCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, AbortCommand)

MESSAGE_CONTENT_DEFINITION_BEGIN
	MachineState state;
MESSAGE_CONTENT_DEFINITION_END(MainController, MachineStateChangedNotification)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int productIndex;
MESSAGE_CONTENT_DEFINITION_END(MainController, ProducingProductNotification)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int ingredientIndex;
	Availability availability;
MESSAGE_CONTENT_DEFINITION_END(MainController, IngredientAvailabilityChangedNotification)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(MainController, InitCommand)
	MESSAGE_CONTENT(MainController, OffCommand)
	MESSAGE_CONTENT(MainController, ProduceProductCommand)
	MESSAGE_CONTENT(MainController, AbortCommand)
	MESSAGE_CONTENT(MainController, MachineStateChangedNotification)
	MESSAGE_CONTENT(MainController, ProducingProductNotification)
	MESSAGE_CONTENT(MainController, IngredientAvailabilityChangedNotification)
MESSAGE_DEFINITION_END(MainController)

extern ActivityDescriptor getMainControllerDescriptor(void);

#endif /* MAINCONTROLLER_H_ */
