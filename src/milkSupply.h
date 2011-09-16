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

COMMON_MESSAGE_CONTENT_REDEFINITION(MilkSupply, InitCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MilkSupply, OffCommand)

MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int milkAmount; // [ml]
MESSAGE_CONTENT_DEFINITION_END(MilkSupply, SupplyMilkCommand)

COMMON_MESSAGE_CONTENT_REDEFINITION(MilkSupply, Result)

MESSAGE_CONTENT_DEFINITION_BEGIN
	Availability availability;
	int milkTemperature;
MESSAGE_CONTENT_DEFINITION_END(MilkSupply, Status)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(MilkSupply, InitCommand)
	MESSAGE_CONTENT(MilkSupply, OffCommand)
	MESSAGE_CONTENT(MilkSupply, SupplyMilkCommand)
	MESSAGE_CONTENT(MilkSupply, Result)
	MESSAGE_CONTENT(MilkSupply, Status)
MESSAGE_DEFINITION_END(MilkSupply)

extern ActivityDescriptor getMilkSupplyDescriptor(void);

#endif /* MILKSUPPLY_H_ */
