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

#include <stdio.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "mainController.h"
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

ActivityDescriptor getMilkSupplyDescriptor() {
	return milkSupply;
}

static void setUpMilkSupply(void *activity) {
	logInfo("[milkSupply] Setting up...");
}

static void runMilkSupply(void *activity) {
	logInfo("[milkSupply] Running...");

	while (TRUE) {
		logInfo("[milkSupply] Going to receive message...");
		MilkSupplyMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[milkSupply] Message received - length: %ld, value: %d, message: %s",
				messageLength, message.intValue, message.strValue);
	}
}

static void tearDownMilkSupply(void *activity) {
	logInfo("[milkSupply] Tearing down...");
}
