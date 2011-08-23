/**
 * @brief   Submodule water supply
 * @file    waterSupply.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Zufuhr: Supply (main Thread)
 */

#include <stdio.h>
#include "sensor.h"
#include "waterSupply.h"

static void setUpWaterSupply(void *activity);
static void runWaterSupply(void *activity);
static void tearDownWaterSupply(void *activity);

static ActivityDescriptor waterSupply = {
	.name = "waterSupply",
	.setUp = setUpWaterSupply,
	.run = runWaterSupply,
	.tearDown = tearDownWaterSupply
};

typedef enum {
	wsState_off = 0,
	wsState_standby,
	wsState_pumpOn,
	wsState_heaterOn
} waterSupplyState;

typedef enum {
	wsEvent_deliverWater = 0,
	wsEvent_hasWater,
	wsEvent_hasFlow,
	wsEvent_hasTemp,
	wsEvent_enoughWater
} waterSupplyEvent;

ActivityDescriptor getWaterSupplyDescriptor() {
	return waterSupply;
}

static void setUpWaterSupply(void *activity) {
	printf("Set up Water supply...\n");
}

static int hasWater(void) {
	return readNonBlockableSensor("./dev/waterSenor");
}

static int hasFlow(void) {
	return readNonBlockableSensor("./dev/waterFlowSenor");
}

static int hasTemp(void) {
	int minTemp = 60;
	int curTemp = readNonBlockableSensor("./dev/waterTemperatureSenor");
	return (curTemp >= minTemp) ? TRUE : FALSE;
}

static void runWaterSupply(void *activity) {
	char buf[80];
	waterSupplyState state = wsState_off;
	waterSupplyEvent event;
	int deliverWater = FALSE;

	while(TRUE) {
		// read message queue:
		// TODO:
		switch(state) {
		case(wsState_off):
			// TODO: ...
			break;
		case(wsState_standby):
			if (deliverWater && hasWater()) {
				if (controlPump(TRUE) < 0) {
					controlPump(FALSE);
					logErr("Could not start water pump!");
				} else {
					state = wsState_pumpOn;
				}
			} else {
				controlPump(FALSE);
			}
			break;
		case(wsState_pumpOn):
			if (deliverWater && hasWater() && hasFlow()) {
				if (controlHeater(TRUE) < 0) {
					controlHeater(FALSE);
					logErr("Could not start heater!");
				} else {
					state = wsState_heaterOn;
				}
			} else {
				controlPump(FALSE);
				controlHeater(FALSE);
				state = wsState_standby;
			}
			break;
		case(wsState_heaterOn):
			if (!deliverWater || !hasWater()) {
				controlPump(FALSE);
				controlHeater(FALSE);
			}
			if (!hasFlow() || hasTemp()) {
				controlHeater(FALSE);
			}
			break
		}
	}

	printf("Running Water supply...\n");

	while (deliverWater) {
		CoffeeMakerEvent;


		break;
		/*
		printf("Going to receive message...\n");
		//char buffer[11];
		//WaterSupplyMessage *message = (WaterSupplyMessage *)buffer;
		WaterSupplyMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		printf("Message received!\n");
		printf("Message length: %ld\n", messageLength);
		printf("Content:\n");
		printf("\tintValue: %d\n", message.intValue);
		printf("\tstringValue: %s\n", message.stringValue);
		*/
	}
}

static void tearDownWaterSupply(void *activity) {
	printf("Tear down Water supply...\n");
}
