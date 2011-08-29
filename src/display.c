/**
 * @brief   <description>
 * @file    display.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 29, 2011
 */

#include <stdio.h>
#include <unistd.h>
#include "defines.h"
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
	printf("[display] Running...\n");

	while(TRUE) {
		printf("[display] Going to receive message...\n");
		msgLen = receiveMessage(activity, (char *)&message, sizeof(message));
		if (msgLen > 0) {
			printf("[display] Message received from %s (length: %d): value: %d, message: %s\n",
				message.activity.name, msgLen, message.intValue, message.strValue);

			// TODO: update display and leds
			sendMessage(getUserInterfaceDescriptor(), (char *)&(UserInterfaceMessage) {
				.activity = getDisplayDescriptor(),
				.intValue = 1,
				.strValue = "Ok, got it!",
			}, sizeof(UserInterfaceMessage), prio_low);
		}
	}
}

static void tearDownDisplay(void *activity) {
	printf("[display] Tearing down...\n");
}
