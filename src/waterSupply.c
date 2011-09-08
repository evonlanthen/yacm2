/**
 * @brief   Submodule water supply
 * @file    waterSupply.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Zufuhr: Supply (main Thread)
 */

#include <stdio.h>
#include <unistd.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "data.h"
#include "stateMachineEngine.h"
#include "mainController.h"
#include "waterSupply.h"

static void setUpWaterSupply(void *activity);
static void runWaterSupply(void *activity);
static void tearDownWaterSupply(void *activity);

static ActivityDescriptor waterSupply = {
	.name = "waterSupply",
	.setUp = setUpWaterSupply,
	.run = runWaterSupply,
	.tearDown = tearDownWaterSupply
};

typedef enum {
	devState_off = 0,
	devState_on
} deviceState;

static StateMachine waterSupplyStateMachine;
static int waterBrewTemperature = 0;
WaterSupplyMessage incomingMessage;
int incomingMessageLength;
void *activity;

ActivityDescriptor getWaterSupplyDescriptor() {
	return waterSupply;
}

static void setUpWaterSupply(void *activity) {
	logInfo("[waterSupply] Setting up...");
}

static int hasWater(void) {
	return readNonBlockingDevice("./dev/waterSensor");
}

static int hasFlow(void) {
	return readNonBlockingDevice("./dev/waterFlowSensor");
}

static int hasTemp(void) {
	int curTemp = readNonBlockingDevice("./dev/waterTemperatureSensor");
	return (curTemp >= waterBrewTemperature) ? TRUE : FALSE;
}

static int checkSensors() {
	if (hasWater() && hasFlow() && hasTemp()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static void controlPump(deviceState state) {
	switch(state) {
	case(devState_on):
		logInfo("[waterSupply] Water pump started");
		break;
	case(devState_off):
		logInfo("[waterSupply] Water pump stopped");
		break;
	}
}

static void controlHeater(deviceState state) {
	switch(state) {
	case(devState_on):
		logInfo("[waterSupply] Water heater started");
		break;
	case(devState_off):
		logInfo("[waterSupply] Water heater stopped");
		break;
	}
}

static void runWaterSupply(void *activityArg) {
	logInfo("[waterSupply] Running...");
	activity = activityArg;
	while(TRUE) {
		// Run state machine
		runStateMachine(&waterSupplyStateMachine);
		sleep(3);
	}
}

/*
 ***************************************************************************
 * States
 ***************************************************************************
 */

// off:
// initializing: betriebsparameter
// idle: bereit; Wassersensor (bei reinit wieder ins initializing)
// supplying; alles einschalten (precondition: hasWater)
/**
 * Represents a water supply state
 */
typedef enum {
	waterSupplyState_switchedOff,
	waterSupplyState_initializing,
	waterSupplyState_idle,
	waterSupplyState_supplying
} WaterSupplyState;

/*
 ***************************************************************************
 * switchedOff state
 ***************************************************************************
 */

static void switchedOffStateEntryAction() {
	;
}

static Event switchedOffStateDoAction() {
	logInfo("[waterSupply] Going to receive message...");
	incomingMessageLength = receiveMessage(activity, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[waterSupply] Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
	return NO_EVENT;
}

static State switchedOffState = {
	.stateIndex = waterSupplyState_switchedOff,
	.entryAction = switchedOffStateEntryAction,
	.doAction = switchedOffStateDoAction
};

/*
 ***************************************************************************
 * initializing state
 ***************************************************************************
 */

static void initializingStateEntryAction() {
	waterBrewTemperature = getMainParameter("waterBrewTemperature");
}

static State initializingState = {
	.stateIndex = waterSupplyState_initializing,
	.entryAction = initializingStateEntryAction
};

/*
 ***************************************************************************
 * idle state
 ***************************************************************************
 */

static void idleStateEntryAction() {
	setMachineState(waterSupplyState_idle);
}

static Event idleStateDoAction() {
	logInfo("[waterSupply] Going to receive message...");
	incomingMessageLength = receiveMessage(activity, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[waterSupply] Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
	return NO_EVENT;
}

static State idleState = {
	.stateIndex = waterSupplyState_idle,
	.entryAction = idleStateEntryAction,
	.doAction = idleStateDoAction
};

/*
 ***************************************************************************
 * supplying state
 ***************************************************************************
 */

static void supplyingStateEntryAction() {
	// set state to supplying:
	setMachineState(waterSupplyState_supplying);

	if (checkSensors()) {
		// start pump and heater:
		controlPump(devState_on);
		controlHeater(devState_on);

		// notifiy mainController:
		sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
			.activity = getWaterSupplyDescriptor(),
			.intValue = 1,
			.strValue = "water supplying started",
		}, sizeof(MainControllerMessage), messagePriority_low);
	} else {
		// notifiy mainController:
		sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
			.activity = getWaterSupplyDescriptor(),
			.intValue = 0,
			.strValue = "could not start water supplying",
		}, sizeof(MainControllerMessage), messagePriority_high);

		// go back to idle state:
		processStateMachineEvent(&waterSupplyStateMachine, waterSupplyEvent_supplyingFinished);
	}
}

static Event supplyingStateDoAction() {
	logInfo("[waterSupply] Going to receive message...");
	incomingMessageLength = receiveMessage(activity, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[waterSupply] Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
	if (!checkSensors()) {
		// stop pump and heater:
		controlPump(devState_off);
		controlHeater(devState_off);

		// notifiy mainController:
		sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
			.activity = getWaterSupplyDescriptor(),
			.intValue = 0,
			.strValue = "had to stop water supplying",
		}, sizeof(MainControllerMessage), messagePriority_high);

		// go back to idle state:
		return waterSupplyEvent_supplyingFinished;
	}
	return NO_EVENT;
}

static void supplyingStateExitAction() {
	// stop pump and heater:
	controlPump(devState_off);
	controlHeater(devState_off);

	// notifiy mainController:
	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
		.activity = getWaterSupplyDescriptor(),
		.intValue = 1,
		.strValue = "water supplying finished",
	}, sizeof(MainControllerMessage), messagePriority_high);
}

static State supplyingState = {
	.stateIndex = waterSupplyState_supplying,
	.entryAction = supplyingStateEntryAction,
	.doAction = supplyingStateDoAction,
	.exitAction = supplyingStateExitAction
};

/*
 ***************************************************************************
 * State transitions
 ***************************************************************************
 */

static StateMachine waterSupplyStateMachine = {
	.numberOfEvents = 6,
	.initialState = &switchedOffState,
	.transitions = {
		/* waterSupplyState_switchedOff: */
			/* waterSupplyEvent_switchOn: */ &initializingState,
			/* waterSupplyEvent_switchOff: */ NULL,
			/* waterSupplyEvent_initialized: */ NULL,
			/* waterSupplyEvent_startSupplying: */ NULL,
			/* waterSupplyEvent_supplyingFinished: */ NULL,
			/* waterSupplyEvent_reconfigure: */ NULL,
		/* waterSupplyState_initializing: */
			/* waterSupplyEvent_switchOn: */ NULL,
			/* waterSupplyEvent_switchOff: */ &switchedOffState,
			/* waterSupplyEvent_initialized: */ &idleState,
			/* waterSupplyEvent_startSupplying: */ NULL,
			/* waterSupplyEvent_supplyingFinished: */ NULL,
			/* waterSupplyEvent_reconfigure: */ NULL,
		/* waterSupplyState_idle: */
			/* waterSupplyEvent_switchOn: */ NULL,
			/* waterSupplyEvent_switchOff: */ &switchedOffState,
			/* waterSupplyEvent_initialized: */ NULL,
			/* waterSupplyEvent_startSupplying: */ &supplyingState,
			/* waterSupplyEvent_supplyingFinished: */ NULL,
			/* waterSupplyEvent_reconfigure: */ &initializingState,
		/* waterSupplyState_supplying: */
			/* waterSupplyEvent_switchOn: */ NULL,
			/* waterSupplyEvent_switchOff: */ &switchedOffState,
			/* waterSupplyEvent_initialized: */ NULL,
			/* waterSupplyEvent_startSupplying: */ NULL,
			/* waterSupplyEvent_supplyingFinished: */ &idleState,
			/* waterSupplyEvent_reconfigure: */ NULL
		}
};

static void tearDownWaterSupply(void *activity) {
	logInfo("[waterSupply] Tearing down...");
}
