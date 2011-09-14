/**
 * @brief   Submodule coffee supply
 * @file    coffeeSupply.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef COFFEESUPPLY_H_
#define COFFEESUPPLY_H_

#include <mqueue.h>
#include "activity.h"

#define POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION 21301
#define POWDER_DISPENSER_NO_BEANS_ERROR 21401
#define POWDER_DISPENSER_START_COMMAND 21001
#define POWDER_DISPENSER_STOP_COMMAND 21002
#define MOTOR_START_COMMAND 22001
#define MOTOR_STOP_COMMAND 22002
#define SUPPLY_START_COMMAND 2001
#define SUPPLY_STOP_COMMAND 2002

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} CoffeeSupplyMessage;

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} CoffeePowderDispenserMessage;

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} FillStateMonitorMessage;

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} MotorControllerMessage;


extern ActivityDescriptor getCoffeeSupplyDescriptor(void);

/**
 * Represents a coffeePowderDispenser event
 */
typedef enum {
	coffeePowderDispenserEvent_init,
	coffeePowderDispenserEvent_switchOff,
	coffeePowderDispenserEvent_initialized,
	coffeePowderDispenserEvent_startSupplying,
	coffeePowderDispenserEvent_supplyingFinished,
	coffeePowderDispenserEvent_stop,
	coffeePowderDispenserEvent_noBeans,
	coffeePowderDispenserEvent_beansAvailable,
} CoffeePowderDispenserEvent;

/**
 * Represents a coffeeSupply event
 */
typedef enum {
	coffeeSupplyEvent_init,
	coffeeSupplyEvent_switchOff,
	coffeeSupplyEvent_initialized,
	coffeeSupplyEvent_startSupplying,
	coffeeSupplyEvent_supplyingFinished,
	coffeeSupplyEvent_stop,
} CoffeeSupplyEvent;


#endif /* COFFEESUPPLY_H_ */
