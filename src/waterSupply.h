/**
 * @brief   Submodule water supply
 * @file    waterSupply.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#include <mqueue.h>
#include "activity.h"

typedef struct {
	int intValue;
	char stringValue[4];
} WaterSupplyMessage;

extern ActivityDescriptor getWaterSupplyDescriptor(void);
