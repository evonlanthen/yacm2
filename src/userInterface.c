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
	logInfo("[userInterface] Setting up...");
	display = createActivity(getDisplayDescriptor(), messageQueue_blocking);
}

static void runUserInterface(void *activity) {
	UserInterfaceMessage uiMsg;
	int msgLen;
	MainControllerMessage mcMsg;
	mcMsg.activity = getUserInterfaceDescriptor();

	logInfo("[userInterface] Running...");

	while (TRUE) {
		logInfo("[userInterface] Going to receive message...");
		msgLen = receiveMessage(activity, (char *)&uiMsg, sizeof(uiMsg));
		logInfo("[userInterface] Message from %s (%d): intVal=%d, strVal='%s'",
				uiMsg.activity.name,
				msgLen,
				uiMsg.intValue,
				uiMsg.strValue);
		if (strcmp(uiMsg.activity.name, "mainController") == 0) {
			logInfo("[userInterface] Send message to display...");
			sendMessage(getDisplayDescriptor(), (char *)&(DisplayMessage) {
				.activity = getUserInterfaceDescriptor(),
				.intValue = 2,
				.strValue = "Show view 2",
			}, sizeof(DisplayMessage), messagePriority_medium);
		} else if (strcmp(uiMsg.activity.name, "display") == 0) {
			logInfo("[userInterface] Send message to mainController (%d, %s)...", uiMsg.intValue, uiMsg.strValue);
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
	logInfo("[userInterface] Tearing down...");
	destroyActivity(display);
}
