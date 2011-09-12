/**
 * @brief   Submodule water supply
 * @file    waterSupply.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef WATERSUPPLY_H_
#define WATERSUPPLY_H_

#include <mqueue.h>
#include "activity.h"

#define SUPPLY_WATER_COMMAND 3001
#define WATER_AVAILABLE_NOTIFICATION 3301
#define NO_WATER_AVAILABLE_ERROR 3401

typedef struct {
	ActivityDescriptor activity;
	int intValue;
	char strValue[256];
} WaterSupplyMessage;

extern ActivityDescriptor getWaterSupplyDescriptor(void);

/**
 * Represents a water supply event
 */
typedef enum {
	waterSupplyEvent_switchOn,
	waterSupplyEvent_switchOff,
	waterSupplyEvent_initialized,
	waterSupplyEvent_startSupplying,
	waterSupplyEvent_supplyingFinished,
	waterSupplyEvent_reconfigure,
} WaterSupplyEvent;

#endif /* WATERSUPPLY_H_ */
