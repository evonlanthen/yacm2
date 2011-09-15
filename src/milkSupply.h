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

typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned int DWord;

MESSAGE_CONTENT_DEFINITION_BEGIN(999, MilkSupplyStatus)
	Byte code;
	char message[128];
MESSAGE_CONTENT_DEFINITION_END

MESSAGE_CONTENT_DEFINITION_BEGIN(999, MilkSupplyCommand)
	Byte command;
MESSAGE_CONTENT_DEFINITION_END

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(MilkSupplyCommand)
	MESSAGE_CONTENT(MilkSupplyStatus)
MESSAGE_DEFINITION_END(MilkSupply)

extern ActivityDescriptor getMilkSupplyDescriptor(void);

#endif /* MILKSUPPLY_H_ */
