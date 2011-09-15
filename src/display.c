/**
 * @brief   <description>
 * @file    display.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 29, 2011
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "activity.h"
#include "userInterface.h"
#include "display.h"

static void setUpDisplay(void *activity);
static void runDisplay(void *activity);
static void tearDownDisplay(void *activity);

static ActivityDescriptor display = {
		.name = "display",
		.setUp = setUpDisplay,
		.run = runDisplay,
		.tearDown = tearDownDisplay
};

ActivityDescriptor getDisplayDescriptor() {
	return display;
}

static void setUpDisplay(void *activity) {
	logInfo("[display] Setting up...");
}

static void runDisplay(void *activity) {
	DisplayMessage displayMessage;
	int messageLength;
	UserInterfaceMessage userInterfaceMessage;
	/*
	userInterfaceMessage.activity = getDisplayDescriptor();

	logInfo("[display] Running...");

	while(TRUE) {
		logInfo("[display] Going to receive message...");
		messageLength = receiveMessage(activity, (char *)&displayMessage, sizeof(displayMessage));
		if (messageLength > 0) {
			logInfo("[display] Message received from %s (length: %d): value: %d, message: %s",
				displayMessage.activity.name, messageLength, displayMessage.intValue, displayMessage.strValue);

			// TODO: update display and leds
			if (writeNonBlockingDevice("./dev/display", displayMessage.strValue, wrm_append, TRUE)) {
				userInterfaceMessage.intValue = TRUE;
				strcpy(userInterfaceMessage.strValue, "Success!");
			} else {
				logErr("[display] Could not write to display!");
				userInterfaceMessage.intValue = FALSE;
				strcpy(userInterfaceMessage.strValue, "Failure!");
			}

			sendMessage(getUserInterfaceDescriptor(),
					(char *)&userInterfaceMessage,
					sizeof(userInterfaceMessage),
					messagePriority_low);
		}
	} */
}

static void tearDownDisplay(void *activity) {
	logInfo("[display] Tearing down...");
}
