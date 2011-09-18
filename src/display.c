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

#define LED_POWER_SWITCH 7
#define LED_WITH_MILK 6
#define LED_INGREDIENTS_MISSING 5
#define LED_WASTE_BIN_FULL 4
#define LED_PRODUCT_3_BUTTON (1<<2)
#define LED_PRODUCT_2_BUTTON (1<<1)
#define LED_PRODUCT_1_BUTTON (1<<0)
#define LED_PRODUCT_BUTTON_BY_INDEX(x) ((x > 0) ? 1<<(x-1) : 0)

static void setUpDisplay(void *activity);
static void runDisplay(void *activity);
static void tearDownDisplay(void *activity);

static ActivityDescriptor display = {
		.name = "display",
		.setUp = setUpDisplay,
		.run = runDisplay,
		.tearDown = tearDownDisplay
};

MESSAGE_CONTENT_TYPE_MAPPING(Display, ChangeViewCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(Display, ShowErrorCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(Display, Result, 3)

static Activity *this = NULL;

ActivityDescriptor getDisplayDescriptor() {
	return display;
}

static void writeDisplay(char *message) {
	if (!writeNonBlockingDevice("./dev/display", message, wrm_append, TRUE)) {
		logErr("[%s] Could not write to display!", this->descriptor->name);
	}
}

static void setUpDisplay(void *activity) {
	//logInfo("[display] Setting up...");

	this = (Activity *)activity;
}

static void runDisplay(void *activity) {
	unsigned int powerState;
	MachineState machineState;
	unsigned int withMilk;
	unsigned int ingredientsMissing;
	unsigned int productIndex;
	unsigned int wasteBinFull;
	int ledsBitField;
	char ledsBitFieldString[5];
	char viewString[301];

	//logInfo("[display] Running...");

	while(TRUE) {
		waitForEvent_BEGIN(this, Display, 1000)
		if (error) {
			//TODO Implement appropriate error handling
			sleep(10);

			// Try again
			continue;
		}
		if (result > 0) {
			MESSAGE_SELECTOR_BEGIN
				MESSAGE_BY_TYPE_SELECTOR(message, Display, ChangeViewCommand)
					//logInfo("[display] Going to change view...");

					powerState = content.powerState;
					machineState = content.machineState;
					withMilk = content.withMilk;
					if (content.coffeeAvailability == available &&
						content.waterAvailability == available &&
						content.milkAvailability == available) {
						ingredientsMissing = FALSE;
					} else {
						ingredientsMissing = TRUE;
					}
					productIndex = content.productIndex;
					wasteBinFull = content.wasteBinFull;
					// setup bitfield:
					ledsBitField = (powerState << LED_POWER_SWITCH);
					ledsBitField += (withMilk << LED_WITH_MILK);
					ledsBitField += (ingredientsMissing << LED_INGREDIENTS_MISSING);
					ledsBitField += LED_PRODUCT_BUTTON_BY_INDEX(productIndex);
					ledsBitField += (wasteBinFull << LED_WASTE_BIN_FULL);
					// write bitfield to string:
					snprintf(ledsBitFieldString, 4, "%d", ledsBitField);
					snprintf(viewString, 300, "New view: powerState=%d, machineState=%d, withMilk=%d, ingredientsMissing=%d, productIndex=%d, wasteBinFull=%d",
						powerState,
						machineState,
						withMilk,
						ingredientsMissing,
						productIndex,
						wasteBinFull);
//					logInfo("[%s] powerState=%d, machineState=%d, withMilk=%d, ingredientsMissing=%d, productIndex=%d, wasteBinFull=%d, ledsBitField=%d, ledBitFieldStr=%s",
//						this->descriptor->name,
//						powerState,
//						machineState,
//						withMilk,
//						ingredientsMissing,
//						productIndex,
//						wasteBinFull,
//						ledsBitField,
//						ledsBitFieldString);
					if (!writeNonBlockingDevice("/dev/leds", ledsBitFieldString, wrm_replace, FALSE)) {
						logErr("[%s] Could not update leds!", this->descriptor->name);
					}

					if (!writeNonBlockingDevice("./dev/display", viewString, wrm_append, TRUE)) {
						logErr("[%s] Could not write to display!", this->descriptor->name);
					}
				MESSAGE_BY_TYPE_SELECTOR(message, Display, ShowErrorCommand)
					writeDisplay(content.message);
			MESSAGE_SELECTOR_END
		}
		waitForEvent_END
	}
}

static void tearDownDisplay(void *activity) {
	//logInfo("[display] Tearing down...");

	if (!writeNonBlockingDevice("/dev/leds", "0", wrm_replace, FALSE)) {
		logErr("[%s] Could not update leds!", this->descriptor->name);
	}
}
