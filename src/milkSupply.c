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
#include "milkSupply.h"

// Message type for milk supply subsystems
//TODO Implement
MESSAGE_DEFINITION_BEGIN
MESSAGE_DEFINITION_END(Subsystem)

// Activity handler prototypes for milk supply
static void setUpMilkSupply(void *activity);
static void runMilkSupply(void *activity);
static void tearDownMilkSupply(void *activity);

// Activity handler prototype for milk supply subsystems
//TODO Implement handlers for each subsystem
static void setUpSubsystem(void *activity);
static void runSubsystem(void *activity);
static void tearDownSubsystem(void *activity);

// Activity descriptors
static ActivityDescriptor milkSupply = {
	.name = "milkSupply",
	.setUp = setUpMilkSupply,
	.run = runMilkSupply,
	.tearDown = tearDownMilkSupply
};

static ActivityDescriptor lacticAcidMonitorDescriptor = {
	.name = "lacticAcidMonitor",
	.setUp = setUpSubsystem,
	.run = runSubsystem,
	.tearDown = tearDownSubsystem
};

static ActivityDescriptor coolingDescriptor = {
	.name = "cooling",
	.setUp = setUpSubsystem,
	.run = runSubsystem,
	.tearDown = tearDownSubsystem
};

static ActivityDescriptor fillStateMonitorDescriptor = {
	.name = "fillStateMonitor",
	.setUp = setUpSubsystem,
	.run = runSubsystem,
	.tearDown = tearDownSubsystem
};

static ActivityDescriptor pipeFlushingDescriptor = {
	.name = "pipeFlushing",
	.setUp = setUpSubsystem,
	.run = runSubsystem,
	.tearDown = tearDownSubsystem
};

static Activity *this;

static Activity *lacticAcidMonitor;
static Activity *cooling;
static Activity *fillStateMonitor;
static Activity *pipeFlushing;

MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, InitCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, OffCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, SupplyMilkCommand, 3)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, Result, 4)
MESSAGE_CONTENT_TYPE_MAPPING(MilkSupply, Status, 5)

ActivityDescriptor getMilkSupplyDescriptor() {
	return milkSupply;
}

static void setUpMilkSupply(void *activity) {
	//logInfo("[milkSupply] Setting up...");

	this = (Activity *)activity;

	lacticAcidMonitor = createActivity(lacticAcidMonitorDescriptor, messageQueue_blocking);
	cooling = createActivity(coolingDescriptor, messageQueue_blocking);
	fillStateMonitor = createActivity(fillStateMonitorDescriptor, messageQueue_blocking);
	pipeFlushing = createActivity(pipeFlushingDescriptor, messageQueue_blocking);
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
				//TODO Implement business logic
				MESSAGE_SELECTOR_BEGIN
					MESSAGE_BY_TYPE_SELECTOR(message, MilkSupply, InitCommand)
						sendResponse_BEGIN(this, MilkSupply, Status)
							.availability = available
						sendResponse_END
						logInfo("[milkSupply] Switched on.");
					MESSAGE_BY_TYPE_SELECTOR(message, MilkSupply, OffCommand)
						logInfo("[milkSupply] Switched off.");
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

	destroyActivity(pipeFlushing);
	destroyActivity(fillStateMonitor);
	destroyActivity(cooling);
	destroyActivity(lacticAcidMonitor);
}

//TODO Implement
static void setUpSubsystem(void *activity) {

}

//TODO Implement
static void runSubsystem(void *activity) {
	while (TRUE) {
		receiveMessage_BEGIN(activity, Subsystem)
		receiveMessage_END
	}
}

//TODO Implement
static void tearDownSubsystem(void *activity) {

}
