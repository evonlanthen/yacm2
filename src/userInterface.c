/**
 * @brief   User interface
 * @file    userInterface.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Benutzerfuehrung: Controller (Main Thread)
 * Ausgabe: Output
 */

#include <stdio.h>
#include "defines.h"
#include "syslog.h"
#include "sensor.h"
#include "mainController.h"
#include "userInterface.h"

static void setUpUserInterface(void *activity);
static void runUserInterface(void *activity);
static void tearDownUserInterface(void *activity);

static ActivityDescriptor userInterface = {
	.name = "userInterface",
	.setUp = setUpUserInterface,
	.run = runUserInterface,
	.tearDown = tearDownUserInterface
};

ActivityDescriptor getUserInterfaceDescriptor() {
	return userInterface;
}

static void setUpUserInterface(void *activity) {
	printf("[userInterface] Setting up...\n");
}

static void runUserInterface(void *activity) {
	printf("[userInterface] Running...\n");

	while (TRUE) {
		printf("[userInterface] Going to receive message...\n");
		UserInterfaceMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		printf("[userInterface] Message received - length: %ld, value: %d, message: %s\n",
				messageLength, message.intValue, message.strValue);
	}
}

static void tearDownUserInterface(void *activity) {
	printf("[userInterface] Tearing down...\n");
}
