/**
 * <short description>
 *
 * <long description>
 *
 * @file    coffeeSupply.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef COFFEESUPPLY_H_
#define COFFEESUPPLY_H_

#include <mqueue.h>
#include "activity.h"

typedef struct {
	int intValue;
	char stringValue[4];
} CoffeeSupplyMessage;

ActivityDescriptor getCoffeeSupplyDescriptor();

#endif /* COFFEESUPPLY_H_ */
