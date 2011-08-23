/**
 * @brief   Submodule coffee supply
 * @file    coffeeSupply.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Mahlwerk: Grinder (main Thread)
 * Kaffeepulverdosierung:  CoffeePowderDispenser
 * Fuellstandsueberwachung: FillStateMonitor
 * Motorsteuerung: MotorController
 */

#include <stdio.h>
#include "syslog.h"
#include "sensor.h"
#include "mainController.h"
#include "coffeeSupply.h"

static void setUpCoffeeSupply(void *activity);
static void runCoffeeSupply(void *activity);
static void tearDownCoffeeSupply(void *activity);

static ActivityDescriptor coffeeSupply = {
	.name = "coffeeSupply",
	.setUp = setUpCoffeeSupply,
	.run = runCoffeeSupply,
	.tearDown = tearDownCoffeeSupply
};

ActivityDescriptor getCoffeeSupplyDescriptor() {
	return coffeeSupply;
}

static void setUpCoffeeSupply(void *activity) {
	printf("Set up coffee supply...\n");
}

static void runCoffeeSupply(void *activity) {
	printf("Running coffee supply...\n");

	while (1) {
		sleep(3);

		printf("Going to receive message...\n");
		//char buffer[11];
		//CoffeeSupplyMessage *message = (CoffeeSupplyMessage *)buffer;
		CoffeeSupplyMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		printf("Message received!\n");
		printf("Message length: %ld\n", messageLength);
		printf("Content:\n");
		printf("\tintValue: %d\n", message.intValue);
		printf("\tstringValue: %s\n", message.stringValue);
	}
}

static void tearDownCoffeeSupply(void *activity) {
	printf("Tear down coffee supply...\n");
}
