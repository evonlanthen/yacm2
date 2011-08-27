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
#include "defines.h"
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
	printf("[coffeeSupply] Setting up...\n");
}

static void runCoffeeSupply(void *activity) {
	printf("[coffeeSupply] Running...\n");

	while (TRUE) {
		printf("[coffeeSupply] Going to receive message...\n");
		CoffeeSupplyMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		printf("[coffeeSupply] Message received - length: %ld, value: %d, message: %s\n",
				messageLength, message.intValue, message.strValue);
	}
}

static void tearDownCoffeeSupply(void *activity) {
	printf("[coffee supply] Tearing down...\n");
}
