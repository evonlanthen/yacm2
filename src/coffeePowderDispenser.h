/*
 * coffeePowderDispenser.h
 *
 *  Created on: Sep 15, 2011
 *      Author: carme
 */

#ifndef COFFEEPOWDERDISPENSER_H_
#define COFFEEPOWDERDISPENSER_H_

#include <mqueue.h>
#include "activity.h"

#define POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION 21301
#define POWDER_DISPENSER_NO_BEANS_ERROR 21401
#define POWDER_DISPENSER_START_COMMAND 21001
#define POWDER_DISPENSER_STOP_COMMAND 21002
#define MOTOR_START_COMMAND 22001
#define MOTOR_STOP_COMMAND 22002

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

extern ActivityDescriptor getCoffeePowderDispenser(void);
extern ActivityDescriptor getCoffeeBeansFillStateMonitor(void);
extern ActivityDescriptor getMotorController(void);


#endif /* COFFEEPOWDERDISPENSER_H_ */
