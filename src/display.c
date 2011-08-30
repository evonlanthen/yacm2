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
	printf("[display] Setting up...\n");
}

static void runDisplay(void *activity) {
	DisplayMessage message;
	int msgLen;
	UserInterfaceMessage uiMsg;
	uiMsg.activity = getDisplayDescriptor();

	printf("[display] Running...\n");

	while(TRUE) {
		printf("[display] Going to receive message...\n");
		msgLen = receiveMessage(activity, (char *)&message, sizeof(message));
		if (msgLen > 0) {
			printf("[display] Message received from %s (length: %d): value: %d, message: %s\n",
				message.activity.name, msgLen, message.intValue, message.strValue);

			// TODO: update display and leds
			if (writeNonBlockableDevice("./dev/display", message.strValue, wrm_append, TRUE)) {
				uiMsg.intValue = TRUE;
				strcpy(uiMsg.strValue, "Success!");
			} else {
				logErr("[display] Could not write to display!");
				uiMsg.intValue = FALSE;
				strcpy(uiMsg.strValue, "Failure!");
			}

			sendMessage(getUserInterfaceDescriptor(),
					(char *)&uiMsg,
					sizeof(uiMsg),
					messagePriority_low);
		}
	}
}

static void tearDownDisplay(void *activity) {
	printf("[display] Tearing down...\n");
}
