/**
 * @brief   Submodule milk supply
 * @file    milkSupply.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Zufuhr: Supply (main Thread)
 * Milchsaeureueberwachung: lacticAcidMonitor
 * Kuehlung: Cooling
 * Fuellstandsueberwachung: FillStateMonitor
 * Leitungsspuehlung: PipeFlushing
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
//#include "mainController.h"
#include "milkSupply.h"

static void setUpMilkSupply(void *activity);
static void runMilkSupply(void *activity);
static void tearDownMilkSupply(void *activity);

static ActivityDescriptor milkSupply = {
	.name = "milkSupply",
	.setUp = setUpMilkSupply,
	.run = runMilkSupply,
	.tearDown = tearDownMilkSupply
};

static Activity *this;

MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, InitCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, SupplyMilkCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, Result, 3)

ActivityDescriptor getMilkSupplyDescriptor() {
	return milkSupply;
}

static void setUpMilkSupply(void *activity) {
	//logInfo("[milkSupply] Setting up...");

	this = (Activity *)activity;
}

static void runMilkSupply(void *activity) {
	//logInfo("[milkSupply] Running...");

	while (TRUE) {
		receiveMessage_BEGIN(this, MilkSupply)
			if (error) {
				//TODO Implement appropriate error handling
				sleep(10);

				// Try again
				continue;
			}
			if (result > 0) {
				MESSAGE_SELECTOR_BEGIN
					MESSAGE_BY_TYPE_SELECTOR(message, MilkSupply, InitCommand)

					MESSAGE_BY_TYPE_SELECTOR(message, MilkSupply, SupplyMilkCommand)
						logInfo("[milkSupply] Supplying %u ml milk...", content.milkAmount);
						logInfo("[milkSupply] ...done.");
						sendResponse_BEGIN(this, MilkSupply, Result)
							.code = OK_RESULT
						sendResponse_END
				MESSAGE_SELECTOR_END
			}
		receiveMessage_END
	}
}

static void tearDownMilkSupply(void *activity) {
	//logInfo("[milkSupply] Tearing down...");
}
