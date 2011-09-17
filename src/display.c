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

#define LED_POWER_SWITCH 1<<7
#define LED_MILK_SELECTOR 1<<6
#define LED_INGREDIENT_STATES 1<<5
#define LED_WASTE_BIN_STATE 1<<4
#define LED_PRODUCT_3_BUTTON 1<<2
#define LED_PRODUCT_2_BUTTON 1<<1
#define LED_PRODUCT_1_BUTTON 1<<0

static void setUpDisplay(void *activity);
static void runDisplay(void *activity);
static void tearDownDisplay(void *activity);

static ActivityDescriptor display = {
		.name = "display",
		.setUp = setUpDisplay,
		.run = runDisplay,
		.tearDown = tearDownDisplay
};

MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, Command, 1)
MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, ChangeViewCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, UpdateLedsCommand, 3)
MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, Status, 4)

static Activity *this;

ActivityDescriptor getDisplayDescriptor() {
	return display;
}

static void setUpDisplay(void *activity) {
	logInfo("[display] Setting up...");
	this = (Activity *)activity;
}

static void runDisplay(void *activity) {
	int viewType;
	char viewString[100] = "Test";

	logInfo("[%s] Running...", (*this->descriptor).name);

	while(TRUE) {
		waitForEvent_BEGIN(this, WaterSupply, 100)
		if (error) {
			//TODO Implement appropriate error handling
			sleep(10);

			// Try again
			continue;
		}
		if (result > 0) {
			MESSAGE_SELECTOR_BEGIN
				MESSAGE_BY_TYPE_SELECTOR(message, Display, ShowViewCommand)
					clientDescriptor = senderDescriptor;
					viewType = content.viewType;
					if (writeNonBlockingDevice("./dev/display", viewString, wrm_append, TRUE)) {
						sendResponse_BEGIN(this, UserInterface, Result)
							.code = OK_RESULT
						sendResponse_END
					} else {
						logErr("[%s] Could not write to display!", (*this->descriptor).name));
						sendResponse_BEGIN(this, UserInterface, Result)
							.code = NOK_RESULT
						sendResponse_END
					}
				MESSAGE_SELECTOR_END
			waitForEvent_END
		}
	}
}

static void tearDownDisplay(void *activity) {
	logInfo("[display] Tearing down...");
}
