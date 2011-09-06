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
#include "device.h"
#include "mainController.h"
#include "coffeeSupply.h"

static void setUpCoffeeSupply(void *activity);
static void runCoffeeSupply(void *activity);
static void tearDownCoffeeSupply(void *activity);
static void setUpCoffeePowderDispenser(void *activity);
static void runCoffeePowderDispenser(void *activity);
static void tearDownCoffeePowderDispenser(void *activity);
static void setUpFillStateMonitor(void *activity);
static void runFillStateMonitor(void *activity);
static void tearDownFillStateMonitor(void *activity);
static void setUpMotorController(void *activity);
static void runMotorController(void *activity);
static void tearDownMotorController(void *activity);

static Activity *coffeePowderDispenser;
static Activity *fillStateMonitor;
static Activity *motorController;

static ActivityDescriptor coffeeSupplyDescriptor = {
	.name = "coffeeSupply",
	.setUp = setUpCoffeeSupply,
	.run = runCoffeeSupply,
	.tearDown = tearDownCoffeeSupply
};

static ActivityDescriptor coffeePowderDispenserDescriptor = {
	.name = "coffeePowderDispenser",
	.setUp = setUpCoffeePowderDispenser,
	.run = runCoffeePowderDispenser,
	.tearDown = tearDownCoffeePowderDispenser
};

static ActivityDescriptor fillStateMonitorDescriptor = {
	.name = "fillStateMonitor",
	.setUp = setUpFillStateMonitor,
	.run = runFillStateMonitor,
	.tearDown = tearDownFillStateMonitor
};

static ActivityDescriptor motorControllerDescriptor = {
	.name = "motorController",
	.setUp = setUpMotorController,
	.run = runMotorController,
	.tearDown = tearDownMotorController
};

ActivityDescriptor getCoffeeSupplyDescriptor() {
	return coffeeSupplyDescriptor;
}

ActivityDescriptor getCoffeePowderDispenser() {
	return coffeePowderDispenserDescriptor;
}

ActivityDescriptor getFillStateMonitor() {
	return fillStateMonitorDescriptor;
}

ActivityDescriptor getMotorController() {
	return motorControllerDescriptor;
}

static void setUpCoffeeSupply(void *activity) {
	logInfo("[coffeeSupply] Setting up...");
	coffeePowderDispenser = createActivity(getCoffeePowderDispenser(), messageQueue_blocking);
}

static void runCoffeeSupply(void *activity) {
	logInfo("[coffeeSupply] Running...");

	while (TRUE) {
		logInfo("[coffeeSupply] Going to receive message...");
		CoffeeSupplyMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[coffeeSupply] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownCoffeeSupply(void *activity) {
	logInfo("[coffee supply] Tearing down...");
}

static void setUpCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Setting up...");
	fillStateMonitor = createActivity(getFillStateMonitor(), messageQueue_blocking);
	motorController = createActivity(getMotorController(), messageQueue_blocking);
}

static void runCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Running...");

	while (TRUE) {
		logInfo("[coffeePowderDispenser] Going to receive message...");
		CoffeePowderDispenserMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[coffeePowderDispenser] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Tearing down...");
}

static void setUpFillStateMonitor(void *activity) {
	logInfo("[fillStateMonitor] Setting up...");
}

static void runFillStateMonitor(void *activity) {
	logInfo("[fillStateMonitor] Running...");

	while (TRUE) {
		logInfo("[fillStateMonitor] Going to receive message...");
		FillStateMonitorMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[fillStateMonitor] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownFillStateMonitor(void *activity) {
	logInfo("[coffeePowderDispenser] Tearing down...");
}

static void setUpMotorController(void *activity) {
	logInfo("[motorController] Setting up...");
}

static void runMotorController(void *activity) {
	logInfo("[motorController] Running...");

	while (TRUE) {
		logInfo("[motorController] Going to receive message...");
		MotorControllerMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[motorController] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownMotorController(void *activity) {
	logInfo("[motorController] Tearing down...");
}
