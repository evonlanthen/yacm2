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

#define SUPPLY_START_COMMAND 2001
#define SUPPLY_STOP_COMMAND 2002
#define SUPPLY_BEANS_AVAILABLE_NOTIFICATION 2301
#define SUPPLY_NO_BEANS_ERROR 2401

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} CoffeeSupplyMessage;

COMMON_MESSAGE_CONTENT_REDEFINITION(CoffeeSupply2, Result)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(CoffeeSupply2, Result)
MESSAGE_DEFINITION_END(CoffeeSupply2)

extern ActivityDescriptor getCoffeeSupplyDescriptor(void);

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
	coffeeSupplyEvent_noBeans,
	coffeeSupplyEvent_beansAvailable,
} CoffeeSupplyEvent;


#endif /* COFFEESUPPLY_H_ */
