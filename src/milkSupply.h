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

MESSAGE_CONTENT_DEFINITION_BEGIN(MilkSupplySupplyMilkCommand)
	unsigned int milkAmount;
MESSAGE_CONTENT_DEFINITION_END

MESSAGE_CONTENT_DEFINITION_BEGIN(MilkSupplyStatus)
	Byte code;
	int milkTemperature;
	char message[128];
MESSAGE_CONTENT_DEFINITION_END

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(MilkSupplySupplyMilkCommand)
	MESSAGE_CONTENT(MilkSupplyStatus)
MESSAGE_DEFINITION_END(MilkSupply)

#define SUPPLY_MILK_COMMAND 1

extern ActivityDescriptor getMilkSupplyDescriptor(void);

#endif /* MILKSUPPLY_H_ */
