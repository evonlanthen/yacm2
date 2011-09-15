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

MESSAGE_CONTENT_TYPE_MAPPING(MilkSupplyCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupplySupplyMilkCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupplyStatus, 9)

ActivityDescriptor getMilkSupplyDescriptor() {
	return milkSupply;
}

static void setUpMilkSupply(void *activity) {
	logInfo("[milkSupply] Setting up...");

	this = (Activity *)activity;
}

static void runMilkSupply(void *activity) {
	logInfo("[milkSupply] Running...");

	while (TRUE) {
		receiveMessage_BEGIN(this, MilkSupply)
			if (result < 0) {
				//TODO Implement appropriate error handling
				sleep(10);

				// Try again
				continue;
			}
			logInfo("[milkSupply] Message received from %s (message length: %u)", senderDescriptor.name, result);
			MESSAGE_SELECTOR_BEGIN
				MESSAGE_SELECTOR_ANY
				//MESSAGE_BY_SENDER_SELECTOR(MainController)
					MESSAGE_SELECTOR_BEGIN
						MESSAGE_BY_TYPE_SELECTOR(message, MilkSupplyCommand)
							logInfo("[milkSupply] Milk supply command received!");
							logInfo("[milkSupply] \tCommand: %u", /* message.content.MilkSupplyCommand. */ content.command);
							sendResponse_BEGIN(this, MilkSupply, MilkSupplyStatus)
								.code = 254
							sendMessage_END(MilkSupply)
						MESSAGE_BY_TYPE_SELECTOR(message, MilkSupplyStatus)
							logInfo("[milkSupply] Milk supply status received!");
							logInfo("[milkSupply] \tCode: %u", /* message.content.MilkSupplyStatus. */ content.code);
							logInfo("[milkSupply] \tMessage: %s", /* message.content.MilkSupplyStatus. */ content.message);
					MESSAGE_SELECTOR_END
				//MESSAGE_SELECTOR_ANY
					//logWarn("[milkSupply] Unexpected message!");
			MESSAGE_SELECTOR_END
		receiveMessage_END
	}
}

static void tearDownMilkSupply(void *activity) {
	logInfo("[milkSupply] Tearing down...");
}
