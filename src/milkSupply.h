/**
 * @brief   Submodule milk supply
 * @file    milkSupply.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef MILKSUPPLY_H_
#define MILKSUPPLY_H_

#include <mqueue.h>
#include "activity.h"

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} MilkSupplyMessage;

extern ActivityDescriptor getMilkSupplyDescriptor(void);

#endif /* MILKSUPPLY_H_ */
