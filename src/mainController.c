/**
 * @brief   Main system control module
 * @file    mainController.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <unistd.h>
#include "sensor.h"
#include "syslog.h"
#include "coffeeSupply.h"
#include "waterSupply.h"
#include "milkSupply.h"
#include "mainController.h"

static void setUpMainController(void *activity);
static void runMainController(void *activity);
static void tearDownMainController(void *activity);

static ActivityDescriptor mainController = {
		.name = "mainController",
		.setUp = setUpMainController,
		.run = runMainController,
		.tearDown = tearDownMainController
};

ActivityDescriptor getMainControllerDescriptor() {
	return mainController;
}

static void setUpMainController(void *activity) {
	printf("Set up main controller...\n");
}

static void runMainController(void *activity) {
	printf("Running main controller...\n");
	logInfo("Running main controller");

	sleep(10);

	printf("Send message to coffee supply...\n");
	printf("Message size: %ld\n", sizeof(CoffeeSupplyMessage));
	sendMessage(getCoffeeSupplyDescriptor(), (char *)&(CoffeeSupplyMessage){
		.intValue = 123,
		.stringValue = "abc"
		}, sizeof(CoffeeSupplyMessage));
	printf("...done. (send message)\n");

	sendMessage(getWaterSupplyDescriptor(), (char *)&(WaterSupplyMessage) {
		.intValue = 42,
		.stringValue = "Start that stuff"
	}, sizeof(WaterSupplyMessage));
}

static void tearDownMainController(void *activity) {
	printf("Tear down main controller...\n");
}
