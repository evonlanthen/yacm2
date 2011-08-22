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
	printf("Set up User Interface...\n");
}

static void runUserInterface(void *activity) {
	printf("Running User Interface...\n");

	while (1) {
		sleep(3);

		printf("Going to receive message...\n");
		//char buffer[11];
		//userInterfaceMessage *message = (userInterfaceMessage *)buffer;
		UserInterfaceMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		printf("Message received!\n");
		printf("Message length: %ld\n", messageLength);
		printf("Content:\n");
		printf("\tintValue: %d\n", message.intValue);
		printf("\tstringValue: %s\n", message.stringValue);
	}
}

static void tearDownUserInterface(void *activity) {
	printf("Tear down User Interface...\n");
}
