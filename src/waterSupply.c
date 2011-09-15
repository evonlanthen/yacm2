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
//#include "mainController.h"
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

MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, InitCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, OffCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, SupplyWaterCommand, 3)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, Result, 4)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, Status, 5)

static Activity *this;

static StateMachine stateMachine;
static int waterBrewTemperature = 0;
static unsigned waterAmountToSupply = 0;

static ActivityDescriptor clientDescriptor;
static ActivityDescriptor callerDescriptor;

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

static int hasWaterState = FALSE;
static int checkWater() {
	static int lastHasWaterState = FALSE;

	hasWaterState = hasWater();
	if (hasWaterState != lastHasWaterState) {
		lastHasWaterState = hasWaterState;

//		if (hasWaterState) {
//			sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
//				.activity = *this->descriptor,
//				.intValue = WATER_AVAILABLE_NOTIFICATION,
//			}, sizeof(MainControllerMessage), messagePriority_high);
			sendNotification_BEGIN(this, WaterSupply, /* getMainControllerDescriptor() */ clientDescriptor, Status)
				.availability = hasWaterState ? available : notAvailable
			sendNotification_END
//		} else {
//			sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
//				.activity = *this->descriptor,
//				.intValue = NO_WATER_AVAILABLE_ERROR
//			}, sizeof(MainControllerMessage), messagePriority_high);
//		}
	}

	return hasWaterState;
}

static void controlPump(DeviceState state) {
	switch(state) {
	case deviceState_on:
		logInfo("[waterSupply] Water pump started");

		writeNonBlockingDevice("./dev/waterPump", "1", wrm_replace, FALSE);

		break;
	case deviceState_off:
		logInfo("[waterSupply] Water pump stopped");

		writeNonBlockingDevice("./dev/waterPump", "0", wrm_replace, FALSE);

		break;
	}
}

static void controlHeater(DeviceState state) {
	switch(state) {
	case deviceState_on:
		logInfo("[waterSupply] Water heater started");

		writeNonBlockingDevice("./dev/waterHeater", "1", wrm_replace, FALSE);

		break;
	case deviceState_off:
		logInfo("[waterSupply] Water heater stopped");

		writeNonBlockingDevice("./dev/waterHeater", "1", wrm_replace, FALSE);

		break;
	}
}

/*
 ***************************************************************************
 * States
 ***************************************************************************
 */

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
 * Switched off state
 ***************************************************************************
 */

static State switchedOffState = {
	.stateIndex = waterSupplyState_switchedOff,
};

/*
 ***************************************************************************
 * Initializing state
 ***************************************************************************
 */

static void initializingStateEntryAction() {
	waterBrewTemperature = getMainParameter("waterBrewTemperature");
}

static Event initializingStateDoAction() {
	return waterSupplyEvent_initialized;
}

static State initializingState = {
	.stateIndex = waterSupplyState_initializing,
	.entryAction = initializingStateEntryAction,
	.doAction = initializingStateDoAction
};

/*
 ***************************************************************************
 * Idle state
 ***************************************************************************
 */

static Event idleStateDoAction() {
	checkWater();

	return NO_EVENT;
}

static State idleState = {
	.stateIndex = waterSupplyState_idle,
	.doAction = idleStateDoAction
};

/*
 ***************************************************************************
 * Supplying state
 ***************************************************************************
 */

static int supplyingStatePrecondition() {
	return hasWaterState;
}

static int isSupplyInitialized;
static SupplyResult supplyResult;
static TIMER supplyInitializingTimer;
static TIMER supplyingTimer;
static void supplyingStateEntryAction() {
	logInfo("[waterSupply] Going to supply %u ml water...", waterAmountToSupply);

	isSupplyInitialized = FALSE;
	supplyResult = supplyResult_nok;

	// Start pump and heater
	controlPump(deviceState_on);
	controlHeater(deviceState_on);

	supplyingTimer = setUpTimer(1000 + (100 * waterAmountToSupply));
	supplyInitializingTimer = setUpTimer(1000);
}

static Event supplyingStateDoAction() {
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

//	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
//		.activity = *this->descriptor,
//		.intValue = (supplyResult == supplyResult_ok ? OK_RESULT : NOK_RESULT),
//		.strValue = "water supplying finished",
//	}, sizeof(MainControllerMessage), messagePriority_high);
	sendNotification_BEGIN(this, WaterSupply, callerDescriptor, Result)
		.code = OK_RESULT
	sendNotification_END
}

static State supplyingState = {
	.stateIndex = waterSupplyState_supplying,
	.precondition = supplyingStatePrecondition,
	.entryAction = supplyingStateEntryAction,
	.doAction = supplyingStateDoAction,
	.exitAction = supplyingStateExitAction
};

/*
 ***************************************************************************
 * State transitions
 ***************************************************************************
 */

static StateMachine stateMachine = {
	.name = "waterSupply",
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
	//logInfo("[waterSupply] Setting up...");

	this = (Activity *)activity;

	setUpStateMachine(&stateMachine);
}

static void runWaterSupply(void *activity) {
	//logInfo("[waterSupply] Running...");

	while (TRUE) {
//		// Wait for incoming message or time event
//		WaterSupplyMessage incomingMessage;
//		int result = waitForEvent(this, (char *)&incomingMessage, sizeof(incomingMessage), 100);
//		if (result < 0) {
//			//TODO Implement appropriate error handling
//			sleep(10);
//
//			// Try again
//			continue;
//		}
//
//		// Check if there is an incoming message
//		if (result > 0) {
//			// Process incoming message
//			switch (0 /* incomingMessage.intValue */) {
//			case INIT_COMMAND:
//				if (stateMachine.activeState == &switchedOffState) {
//					processStateMachineEvent(&stateMachine, waterSupplyEvent_switchOn);
//				} else {
//					processStateMachineEvent(&stateMachine, waterSupplyEvent_reconfigure);
//				}
//
//				break;
//			case OFF_COMMAND:
//				processStateMachineEvent(&stateMachine, waterSupplyEvent_switchOff);
//
//				break;
//			case SUPPLY_WATER_COMMAND:
//				processStateMachineEvent(&stateMachine, waterSupplyEvent_startSupplying);
//
//				break;
//			}
//		}
		waitForEvent_BEGIN(this, WaterSupply, 100)
			if (error) {
				//TODO Implement appropriate error handling
				sleep(10);

				// Try again
				continue;
			}
			if (result > 0) {
				MESSAGE_SELECTOR_BEGIN
					MESSAGE_BY_TYPE_SELECTOR(message, WaterSupply, InitCommand)
						clientDescriptor = senderDescriptor;

						if (stateMachine.activeState == &switchedOffState) {
							processStateMachineEvent(&stateMachine, waterSupplyEvent_switchOn);
						} else {
							processStateMachineEvent(&stateMachine, waterSupplyEvent_reconfigure);
						}
					MESSAGE_BY_TYPE_SELECTOR(message, WaterSupply, OffCommand)
						processStateMachineEvent(&stateMachine, waterSupplyEvent_switchOff);
					MESSAGE_BY_TYPE_SELECTOR(message, WaterSupply, SupplyWaterCommand)
						if (stateMachine.activeState == &idleState) {
							callerDescriptor = senderDescriptor;

							waterAmountToSupply = content.waterAmount;

							processStateMachineEvent(&stateMachine, waterSupplyEvent_startSupplying);
						} else {
							sendResponse_BEGIN(this, WaterSupply, Result)
								.code = NOK_RESULT
							sendResponse_END
						}
				MESSAGE_SELECTOR_END
			}
		waitForEvent_END

		// Run state machine
		runStateMachine(&stateMachine);
	}
}

static void tearDownWaterSupply(void *activity) {
	logInfo("[waterSupply] Tearing down...");

	abortStateMachine(&stateMachine);
}
