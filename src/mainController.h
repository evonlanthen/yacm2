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

#define PROCESS_NO_ACTIVITY 0
#define PROCESS_WARMING_UP_ACTIVITY 1
#define	PROCESS_CHECKING_CUP_FILL_STATE_ACTIVITY 2
#define PROCESS_GRINDING_COFFEE_POWDER_ACTIVITY 3
#define PROCESS_SUPPLYING_WATER_ACTIVITY 4
#define PROCESS_SUPPLYING_MILK_ACTIVITY 5
#define PROCESS_EJECTING_COFFEE_WASTE_ACTIVITY 6

#define PROCESS_CUP_IS_NOT_EMPTY_ERROR 1
#define PROCESS_NO_COFFEE_BEANS_ERROR 2
#define PROCESS_COFFEE_WASTE_BIN_IS_FULL_ERROR 3
#define PROCESS_NO_WATER_ERROR 4
#define PROCESS_NO_WATER_FLOW_ERROR 5
#define PROCESS_WATER_TEMPERATURE_TOO_LOW_ERROR 6
#define PROCESS_NO_MILK_ERROR 7
#define PROCESS_UNDEFINED_PRODUCT_ERROR 8

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, InitCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, OffCommand)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int productIndex;
	int withMilk;
MESSAGE_CONTENT_DEFINITION_END(MainController, ProduceProductCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, AbortCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MainController, Result)

MESSAGE_CONTENT_DEFINITION_BEGIN
	MachineState state;
MESSAGE_CONTENT_DEFINITION_END(MainController, MachineStateChangedNotification)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int productIndex;
MESSAGE_CONTENT_DEFINITION_END(MainController, ProducingProductNotification)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int activityIndex;
MESSAGE_CONTENT_DEFINITION_END(MainController, ExecutingActivityNotification)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int ingredientIndex;
	Availability availability;
MESSAGE_CONTENT_DEFINITION_END(MainController, IngredientAvailabilityChangedNotification)

MESSAGE_CONTENT_DEFINITION_BEGIN
	int isBinFull;
MESSAGE_CONTENT_DEFINITION_END(MainController, CoffeeWasteBinStateChangedNotification)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(MainController, InitCommand)
	MESSAGE_CONTENT(MainController, OffCommand)
	MESSAGE_CONTENT(MainController, ProduceProductCommand)
	MESSAGE_CONTENT(MainController, AbortCommand)
	MESSAGE_CONTENT(MainController, Result)
	MESSAGE_CONTENT(MainController, MachineStateChangedNotification)
	MESSAGE_CONTENT(MainController, ProducingProductNotification)
	MESSAGE_CONTENT(MainController, ExecutingActivityNotification)
	MESSAGE_CONTENT(MainController, IngredientAvailabilityChangedNotification)
	MESSAGE_CONTENT(MainController, CoffeeWasteBinStateChangedNotification)
MESSAGE_DEFINITION_END(MainController)

extern ActivityDescriptor getMainControllerDescriptor(void);

#endif /* MAINCONTROLLER_H_ */
