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

static ActivityDescriptor userInterfaceDescriptor = {
	.name = "userInterface",
	.setUp = setUpUserInterface,
	.run = runUserInterface,
	.tearDown = tearDownUserInterface
};

ActivityDescriptor getUserInterfaceDescriptor() {
	return userInterfaceDescriptor;
}

static void setUpUserInterface(void *activity) {
	logInfo("[userInterface] Setting up...");
	display = createActivity(getDisplayDescriptor(), messageQueue_blocking);
}

static void runUserInterface(void *activity) {
	UserInterfaceMessage incomingMessage;
	int incomingMessageLength;
	MainControllerMessage mainControllerMessage;
	mainControllerMessage.activity = getUserInterfaceDescriptor();

	logInfo("[userInterface] Running...");

	while (TRUE) {
		waitForEvent(activity, (char *)&incomingMessage, sizeof(incomingMessage), 100);

		MESSAGE_SELECTOR_BEGIN
		MESSAGE_SELECTOR(incomingMessage, mainController)
			logInfo("[userInterface] Send message to display...");
			sendMessage(getDisplayDescriptor(), (char *)&(DisplayMessage) {
				.activity = getUserInterfaceDescriptor(),
				.intValue = 2,
				.strValue = "Show view 2",
			}, sizeof(DisplayMessage), messagePriority_medium);
		MESSAGE_SELECTOR(incomingMessage, display)
			logInfo("[userInterface] Send message to mainController (%d, %s)...", incomingMessage.intValue, incomingMessage.strValue);
			mainControllerMessage.intValue = incomingMessage.intValue;
			strcpy(mainControllerMessage.strValue, incomingMessage.strValue);
			sendMessage(getMainControllerDescriptor(),
				(char *)&mainControllerMessage,
				sizeof(mainControllerMessage),
				messagePriority_medium);
		MESSAGE_SELECTOR_END
	}
}

static void tearDownUserInterface(void *activity) {
	logInfo("[userInterface] Tearing down...");
	destroyActivity(display);
}
