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

typedef struct {
	int intValue;
	char strValue[256];
} CoffeeSupplyMessage;

extern ActivityDescriptor getCoffeeSupplyDescriptor(void);

#endif /* COFFEESUPPLY_H_ */
