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

ActivityDescriptor getWaterSupplyDescriptor() {
	return waterSupply;
}

static void setUpWaterSupply(void *activity) {
	printf("Set up Water supply...\n");
}

static void runWaterSupply(void *activity) {
	printf("Running Water supply...\n");

	while (1) {
		sleep(3);

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
	}
}

static void tearDownWaterSupply(void *activity) {
	printf("Tear down Water supply...\n");
}
