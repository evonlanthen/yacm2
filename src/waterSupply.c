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
#include "waterSupply.h"

typedef enum {
	supplyResult_ok,
	supplyResult_nok
} SupplyResult;

// Activity handler prototypes
static void setUpWaterSupply(void *activity);
static void runWaterSupply(void *activity);
static void tearDownWaterSupply(void *activity);

// Activity descriptor
static ActivityDescriptor waterSupply = {
	.name = "waterSupply",
	.setUp = setUpWaterSupply,
	.run = runWaterSupply,
	.tearDown = tearDownWaterSupply
};

MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, InitCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, OffCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, SupplyWaterCommand, 3)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, AbortCommand, 4)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, Result, 5)
MESSAGE_CONTENT_TYPE_MAPPING(WaterSupply, Status, 6)

static Activity *this;

static StateMachine stateMachine;
static int waterBrewTemperature = 0;
static unsigned int waterAmountToSupply = 0;

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

		sendNotification_BEGIN(this, WaterSupply, /* getMainControllerDescriptor() */ clientDescriptor, Status)
			.availability = hasWaterState ? available : notAvailable
		sendNotification_END
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
 * Events
 ***************************************************************************
 */

/**
 * Represents a water supply event
 */
typedef enum {
	waterSupplyEvent_switchOn,
	waterSupplyEvent_switchOff,
	waterSupplyEvent_initialized,
	waterSupplyEvent_startSupplying,
	waterSupplyEvent_supplyingFinished,
	waterSupplyEvent_reconfigure,
} WaterSupplyEvent;

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
static int supplyError;
static TIMER supplyInitializingTimer;
static TIMER supplyingTimer;
static void supplyingStateEntryAction() {
	logInfo("[waterSupply] Going to supply %u ml water with a temperature of %d °C...", waterAmountToSupply, waterBrewTemperature);

	isSupplyInitialized = FALSE;
	supplyResult = supplyResult_nok;
	supplyError = NO_ERROR;

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
		supplyError = NO_WATER_ERROR;

		return waterSupplyEvent_supplyingFinished;
	}

	if (isSupplyInitialized) {
		// Check flow and temperature
		if (!hasFlow()) {
			supplyError = NO_WATER_FLOW_ERROR;

			return waterSupplyEvent_supplyingFinished;
		}
		if (getTemperature() < waterBrewTemperature) {
			supplyError = WATER_TEMPERATURE_TOO_LOW_ERROR;

			return waterSupplyEvent_supplyingFinished;
		}
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

	logInfo("[waterSupply] ...done (supplying water).");

	sendNotification_BEGIN(this, WaterSupply, callerDescriptor, Result)
		.code = supplyResult == supplyResult_ok ? OK_RESULT : NOK_RESULT,
		.errorCode = supplyError
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
					MESSAGE_BY_TYPE_SELECTOR(message, WaterSupply, AbortCommand)
						supplyError = ABORTED_ERROR;

						processStateMachineEvent(&stateMachine, waterSupplyEvent_supplyingFinished);
				MESSAGE_SELECTOR_END
			}
		waitForEvent_END

		// Run state machine
		runStateMachine(&stateMachine);
	}
}

static void tearDownWaterSupply(void *activity) {
	//logInfo("[waterSupply] Tearing down...");

	abortStateMachine(&stateMachine);
}
