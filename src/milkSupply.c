/**
 * @brief   Submodule milk supply
 * @file    milkSupply.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Zufuhr: Supply (main Thread)
 * Milchsaeureueberwachung: lacticAcidMonitor
 * Kuehlung: Cooling
 * Fuellstandsueberwachung: FillStateMonitor
 * Leitungsspuehlung: PipeFlushing
 */

#include <stdio.h>
#include "syslog.h"
#include "milkSupply.h"

static void setUpMilkSupply(void *activity);
static void runMilkSupply(void *activity);
static void tearDownMilkSupply(void *activity);

static ActivityDescriptor milkSupply = {
	.name = "milkSupply",
	.setUp = setUpMilkSupply,
	.run = runMilkSupply,
	.tearDown = tearDownMilkSupply
};

ActivityDescriptor getMilkSupplyDescriptor() {
	return milkSupply;
}

static void setUpMilkSupply(void *activity) {
	printf("Set up milk supply...\n");
}

static void runMilkSupply(void *activity) {
	printf("Running milk supply...\n");

	while (1) {
		sleep(3);

		printf("Going to receive message...\n");
		//char buffer[11];
		//milkSupplyMessage *message = (milkSupplyMessage *)buffer;
		MilkSupplyMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		printf("Message received!\n");
		printf("Message length: %ld\n", messageLength);
		printf("Content:\n");
		printf("\tintValue: %d\n", message.intValue);
		printf("\tstringValue: %s\n", message.stringValue);
	}
}

static void tearDownMilkSupply(void *activity) {
	printf("Tear down milk supply...\n");
}
