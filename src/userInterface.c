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
#include <string.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "display.h"
#include "mainController.h"
#include "userInterface.h"

static void setUpUserInterface(void *activity);
static void runUserInterface(void *activity);
static void tearDownUserInterface(void *activity);

static Activity *display;

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
	display = createActivity(getDisplayDescriptor(), messageQueue_blocking);
}

static void runUserInterface(void *activity) {
	UserInterfaceMessage uiMsg;
	int msgLen;
	MainControllerMessage mcMsg;
	mcMsg.activity = getUserInterfaceDescriptor();

	printf("[userInterface] Running...\n");

	while (TRUE) {
		printf("[userInterface] Going to receive message...\n");
		msgLen = receiveMessage(activity, (char *)&uiMsg, sizeof(uiMsg));
		printf("[userInterface] Message from %s (%d): intVal=%d, strVal='%s'\n",
				uiMsg.activity.name,
				msgLen,
				uiMsg.intValue,
				uiMsg.strValue);
		if (strcmp(uiMsg.activity.name, "mainController") == 0) {
			printf("[userInterface] Send message to display...\n");
			sendMessage(getDisplayDescriptor(), (char *)&(DisplayMessage) {
				.activity = getUserInterfaceDescriptor(),
				.intValue = 2,
				.strValue = "Show view 2",
			}, sizeof(DisplayMessage), messagePriority_medium);
		} else if (strcmp(uiMsg.activity.name, "display") == 0) {
			printf("[userInterface] Send message to mainController (%d, %s)...\n", uiMsg.intValue, uiMsg.strValue);
			mcMsg.intValue = uiMsg.intValue;
			strcpy(mcMsg.strValue, uiMsg.strValue);
			sendMessage(getMainControllerDescriptor(),
				(char *)&mcMsg,
				sizeof(mcMsg),
				messagePriority_medium);
		}
	}
}

static void tearDownUserInterface(void *activity) {
	printf("[userInterface] Tearing down...\n");
	destroyActivity(display);
}
