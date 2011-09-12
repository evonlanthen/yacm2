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
#include "timer.h"
#include "device.h"
#include "data.h"
#include "stateMachineEngine.h"
#include "mainController.h"
#include "waterSupply.h"

typedef enum {
	supplyResult_ok,
	supplyResult_nok
} SupplyResult;

static void setUpWaterSupply(void *activity);
static void runWaterSupply(void *activity);
static void tearDownWaterSupply(void *activity);

static ActivityDescriptor waterSupply = {
	.name = "waterSupply",
	.setUp = setUpWaterSupply,
	.run = runWaterSupply,
	.tearDown = tearDownWaterSupply
};

static Activity *this;

static StateMachine waterSupplyStateMachine;
static int waterBrewTemperature = 0;
//WaterSupplyMessage incomingMessage;
//int incomingMessageLength;

ActivityDescriptor getWaterSupplyDescriptor() {
	return waterSupply;
}

static int hasWater() {
	if (readNonBlockingDevice("./dev/waterSensor") > 0) {
		return TRUE;
	}

	return FALSE;
}

static int hasFlow() {
	if (readNonBlockingDevice("./dev/waterFlowSensor") > 0) {
		return TRUE;
	}

	return FALSE;
}

static int getTemperature() {
	return readNonBlockingDevice("./dev/waterTemperatureSensor");
}

static int lastHasWaterState = FALSE;
static int checkWater() {
	int hasWaterState = hasWater();
	if (hasWaterState != lastHasWaterState) {
		lastHasWaterState = hasWaterState;

		if (hasWaterState) {
			sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
				.activity = *this->descriptor,
				.intValue = WATER_AVAILABLE_NOTIFICATION,
			}, sizeof(MainControllerMessage), messagePriority_high);
		} else {
			sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
				.activity = *this->descriptor,
				.intValue = NO_WATER_AVAILABLE_ERROR
			}, sizeof(MainControllerMessage), messagePriority_high);
		}
	}

	return hasWaterState;
}

/*
static int checkSensors() {
	if (hasWater() && hasFlow() && hasTemp()) {
		return TRUE;
	} else {
		return FALSE;
	}
}
*/

static void controlPump(DeviceState state) {
	switch(state) {
	case(deviceState_on):
		logInfo("[waterSupply] Water pump started");
		break;
	case(deviceState_off):
		logInfo("[waterSupply] Water pump stopped");
		break;
	}
}

static void controlHeater(DeviceState state) {
	switch(state) {
	case(deviceState_on):
		logInfo("[waterSupply] Water heater started");
		break;
	case(deviceState_off):
		logInfo("[waterSupply] Water heater stopped");
		break;
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

/*
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
*/

static State switchedOffState = {
	.stateIndex = waterSupplyState_switchedOff,
	//.doAction = switchedOffStateDoAction
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

/*
static void idleStateEntryAction() {
	setMachineState(waterSupplyState_idle);
}
*/

static Event idleStateDoAction() {
	/*
	logInfo("[waterSupply] Going to receive message...");
	incomingMessageLength = receiveMessage(activity, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[waterSupply] Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
	*/
	checkWater();

	return NO_EVENT;
}

static State idleState = {
	.stateIndex = waterSupplyState_idle,
	//.entryAction = idleStateEntryAction,
	.doAction = idleStateDoAction
};

/*
 ***************************************************************************
 * supplying state
 ***************************************************************************
 */

static int isSupplyInitialized;
static SupplyResult supplyResult;
static TIMER supplyInitializingTimer;
static TIMER supplyingTimer;
static void supplyingStateEntryAction() {
	// set state to supplying:
	//setMachineState(waterSupplyState_supplying);

	/*
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
	*/

	isSupplyInitialized = FALSE;
	supplyResult = supplyResult_nok;

	// Start pump and heater
	controlPump(deviceState_on);
	controlHeater(deviceState_on);

	supplyingTimer = setUpTimer(10000);
	supplyInitializingTimer = setUpTimer(1000);
}

static Event supplyingStateDoAction() {
	/*
	logInfo("[waterSupply] Going to receive message...");
	incomingMessageLength = receiveMessage(activity, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[waterSupply] Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
	*/

	if (isTimerElapsed(supplyInitializingTimer)) {
		supplyInitializingTimer = NULL;

		isSupplyInitialized = TRUE;
	}

	// Check water
	if (!checkWater()) {
		return waterSupplyEvent_supplyingFinished;
	}

	if (isSupplyInitialized
			// Check flow and temperature
			&& (!hasFlow() || getTemperature() < waterBrewTemperature)) {
		return waterSupplyEvent_supplyingFinished;
	}

	if (isTimerElapsed(supplyingTimer)) {
		supplyingTimer = NULL;

		supplyResult = supplyResult_ok;

		return waterSupplyEvent_supplyingFinished;
	}

	/*
	if (!checkSensors()) {
		// Stop pump and heater:
		controlPump(deviceState_off);
		controlHeater(deviceState_off);

		// notifiy mainController:
		sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
			.activity = *this->descriptor,
			.intValue = 0,
			.strValue = "had to stop water supplying",
		}, sizeof(MainControllerMessage), messagePriority_high);

		// go back to idle state:
		return waterSupplyEvent_supplyingFinished;
	}
	*/

	return NO_EVENT;
}

static void supplyingStateExitAction() {
	if (supplyInitializingTimer) {
		abortTimer(supplyInitializingTimer);
	}
	if (supplyingTimer) {
		abortTimer(supplyingTimer);
	}

	// Stop pump and heater
	controlPump(deviceState_off);
	controlHeater(deviceState_off);

	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
		.activity = *this->descriptor,
		.intValue = (supplyResult == supplyResult_ok ? OK_RESULT : NOK_RESULT),
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

static void setUpWaterSupply(void *activity) {
	logInfo("[waterSupply] Setting up...");

	this = (Activity *)activity;
}

static void runWaterSupply(void *activity) {
	logInfo("[waterSupply] Running...");

	while(TRUE) {
		// Wait for incoming message or time event
		WaterSupplyMessage incomingMessage;
		int result = waitForEvent(this, (char *)&incomingMessage, sizeof(incomingMessage), 1000);
		if (result < 0) {
			//TODO Implement apropriate error handling
			sleep(10);

			// Try to recover from error
			continue;
		}

		// Check if there is an incoming message
		if (result > 0) {
			// Process incoming message
			switch (incomingMessage.intValue) {
			case INIT_COMMAND:
				if (waterSupplyStateMachine.activeState == &switchedOffState) {
					processStateMachineEvent(&waterSupplyStateMachine, waterSupplyEvent_switchOn);
				} else {
					processStateMachineEvent(&waterSupplyStateMachine, waterSupplyEvent_reconfigure);
				}

				break;
			case OFF_COMMAND:
				processStateMachineEvent(&waterSupplyStateMachine, waterSupplyEvent_switchOff);

				break;
			case SUPPLY_WATER_COMMAND:
				processStateMachineEvent(&waterSupplyStateMachine, waterSupplyEvent_startSupplying);

				break;
			}
		}

		// Run state machine
		runStateMachine(&waterSupplyStateMachine);
	}
}

static void tearDownWaterSupply(void *activity) {
	logInfo("[waterSupply] Tearing down...");
}
