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

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int milkAmount;
MESSAGE_CONTENT_DEFINITION_END(MilkSupply, SupplyMilkCommand)

MESSAGE_CONTENT_DEFINITION_BEGIN
	Byte code;
	int milkTemperature;
	char message[128];
MESSAGE_CONTENT_DEFINITION_END(MilkSupply, Status)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(MilkSupply, SupplyMilkCommand)
	MESSAGE_CONTENT(MilkSupply, Status)
MESSAGE_DEFINITION_END(MilkSupply)

#define SUPPLY_MILK_COMMAND 1

extern ActivityDescriptor getMilkSupplyDescriptor(void);

#endif /* MILKSUPPLY_H_ */
