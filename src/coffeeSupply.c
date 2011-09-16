/**
 * @brief   Submodule coffee supply
 * @file    coffeeSupply.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Mahlwerk: Grinder (main Thread)
 * Kaffeepulverdosierung:  CoffeePowderDispenser
 * Fuellstandsueberwachung: FillStateMonitor
 * Motorsteuerung: MotorController
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "mainController.h"
#include "coffeeSupply.h"
#include "coffeePowderDispenser.h"
#include "activity.h"
#include "stateMachineEngine.h"

static void setUpCoffeeSupply(void *activity);
static void runCoffeeSupply(void *activity);
static void tearDownCoffeeSupply(void *activity);

static StateMachine coffeeSupplyStateMachine;

static Activity *coffeeSupply;
static Activity *coffeePowderDispenser;

static ActivityDescriptor coffeeSupplyDescriptor = {
	.name = "coffeeSupply",
	.setUp = setUpCoffeeSupply,
	.run = runCoffeeSupply,
	.tearDown = tearDownCoffeeSupply
};

MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply2, GrindCoffeePowderCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply2, EjectCoffeeWasteCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply2, Result, 3)

static int ejectWaste(void) {
	return writeNonBlockingDevice("/dev/coffeeWasteEjector","1",wrm_replace,FALSE);
}

static int lastHasCoffeeWasteState = TRUE;

static int hasCoffeeWaste(void) {
	return readNonBlockingDevice("./dev/coffeeWasteSensor");
}

// coffeeSupply state for beans
static int lastHasBeans = TRUE;


/*
 ***************************************************************************
 * States coffeeSupply
 ***************************************************************************
 */

// off:
// initializing:
// idle: bereit;
// supplying;
/**
 * Represents a coffee supply state
 */
typedef enum {
	coffeeSupplyState_switchedOff,
	coffeeSupplyState_initializing,
	coffeeSupplyState_idle,
	coffeeSupplyState_supplying
} CoffeeSupplyState;

/*
 ***************************************************************************
 * switchedOff state
 ***************************************************************************
 */

static void coffeeSupplySwitchedOffStateEntryAction() {
	logInfo("[coffeeSupply] Entered SwitchedOff State...");
}

static Event coffeeSupplySwitchedOffStateDoAction() {
	return NO_EVENT;
}

static State coffeeSupplySwitchedOffState = {
	.stateIndex = coffeeSupplyState_switchedOff,
	.entryAction = coffeeSupplySwitchedOffStateEntryAction,
	.doAction = coffeeSupplySwitchedOffStateDoAction
};

/*
 ***************************************************************************
 * initializing state
 ***************************************************************************
 */

static void coffeeSupplyInitializingStateEntryAction() {
	logInfo("[coffeeSupply] Send message to coffeePowderDispenser...");
	//Send init message to powder dispenser
	sendMessage(getCoffeePowderDispenser(), (char *)&(CoffeePowderDispenserMessage){
			.activity = getCoffeeSupplyDescriptor(),
			.intValue = INIT_COMMAND,
			.strValue = "init coffeePowderDispenser"
			}, sizeof(CoffeePowderDispenserMessage), messagePriority_medium);
	logInfo("[coffeeSupply] ...done. (send message)");
}

static Event coffeeSupplyInitializingStateDoAction() {
	sleep(1);
	return coffeeSupplyEvent_initialized;
}


static State coffeeSupplyInitializingState = {
	.stateIndex = coffeeSupplyState_initializing,
	.entryAction = coffeeSupplyInitializingStateEntryAction,
	.doAction = coffeeSupplyInitializingStateDoAction
};

/*
 ***************************************************************************
 * idle state
 ***************************************************************************
 */

static void coffeeSupplyIdleStateEntryAction() {
	;
}

static Event coffeeSupplyIdleStateDoAction() {

	return NO_EVENT;
}

static State coffeeSupplyIdleState = {
	.stateIndex = coffeeSupplyState_idle,
	.entryAction = coffeeSupplyIdleStateEntryAction,
	.doAction = coffeeSupplyIdleStateDoAction
};

/*
 ***************************************************************************
 * supplying state
 ***************************************************************************
 */

static void coffeeSupplySupplyingStateEntryAction() {
	// notifiy mainController:
//	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
//		.activity = getCoffeeSupplyDescriptor(),
//		.intValue = 1,
//		.strValue = "coffee powder supplying started",
//	}, sizeof(MainControllerMessage), messagePriority_low);
}

static Event coffeeSupplySupplyingStateDoAction() {
	return NO_EVENT;
}

static void coffeeSupplySupplyingStateExitAction() {
	// eject Waste
	ejectWaste();
	// notifiy mainController:
//	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
//		.activity = getCoffeeSupplyDescriptor(),
//		.intValue = OK_RESULT,
//		.strValue = "coffee powder supplying finished",
//	}, sizeof(MainControllerMessage), messagePriority_high);
	sendNotification_BEGIN(coffeeSupply, CoffeeSupply2, getMainControllerDescriptor(), Result)
		.code = OK_RESULT
	sendNotification_END
}

static State coffeeSupplySupplyingState = {
	.stateIndex = coffeeSupplyState_supplying,
	.entryAction = coffeeSupplySupplyingStateEntryAction,
	.doAction = coffeeSupplySupplyingStateDoAction,
	.exitAction = coffeeSupplySupplyingStateExitAction
};

/*
 ***************************************************************************
 * State transitions
 ***************************************************************************
 */

static StateMachine coffeeSupplyStateMachine = {
	.numberOfEvents = 6,
	.initialState = &coffeeSupplySwitchedOffState,
	.transitions = {
		/* coffeeSupplyState_switchedOff: */
			/* coffeeSupplyEvent_init: */ &coffeeSupplyInitializingState,
			/* coffeeSupplyEvent_switchOff: */ NULL,
			/* coffeeSupplyEvent_initialized: */ NULL,
			/* coffeeSupplyEvent_startSupplying: */ NULL,
			/* coffeeSupplyEvent_supplyingFinished: */ NULL,
			/* coffeeSupplyEvent_stop: */ NULL,
			/* coffeeSupplyEvent_noBeans: */ NULL,
			/* coffeeSupplyEvent_beansAvailable: */ NULL,
		/* coffeeSupplyState_initializing: */
			/* coffeeSupplyEvent_init: */ NULL,
			/* coffeeSupplyEvent_switchOff: */ &coffeeSupplySwitchedOffState,
			/* coffeeSupplyEvent_initialized: */ &coffeeSupplyIdleState,
			/* coffeeSupplyEvent_startSupplying: */ NULL,
			/* coffeeSupplyEvent_supplyingFinished: */ NULL,
			/* coffeeSupplyEvent_stop: */ NULL,
			/* coffeeSupplyEvent_noBeans: */ NULL,
			/* coffeeSupplyEvent_beansAvailable: */ NULL,
		/* coffeeSupplyState_idle: */
			/* coffeeSupplyEvent_init: */ &coffeeSupplyInitializingState,
			/* coffeeSupplyEvent_switchOff: */ &coffeeSupplySwitchedOffState,
			/* coffeeSupplyEvent_initialized: */ NULL,
			/* coffeeSupplyEvent_startSupplying: */ &coffeeSupplySupplyingState,
			/* coffeeSupplyEvent_supplyingFinished: */ NULL,
			/* coffeeSupplyEvent_stop: */ NULL,
			/* coffeeSupplyEvent_noBeans: */ NULL,
			/* coffeeSupplyEvent_beansAvailable: */ NULL,
		/* coffeeSupplyState_supplying: */
			/* coffeeSupplyEvent_init: */ NULL,
			/* coffeeSupplyEvent_switchOff: */ &coffeeSupplySwitchedOffState,
			/* coffeeSupplyEvent_initialized: */ NULL,
			/* coffeeSupplyEvent_startSupplying: */ NULL,
			/* coffeeSupplyEvent_supplyingFinished: */ &coffeeSupplyIdleState,
			/* coffeeSupplyEvent_stop: */ &coffeeSupplyIdleState,
			/* coffeeSupplyEvent_noBeans: */ &coffeeSupplyIdleState,
			/* coffeeSupplyEvent_beansAvailable: */ NULL
		}
};

ActivityDescriptor getCoffeeSupplyDescriptor() {
	return coffeeSupplyDescriptor;
}

static void setUpCoffeeSupply(void *activityarg) {
	logInfo("[coffeeSupply] Setting up...");
	coffeeSupply = activityarg;
	setUpStateMachine(&coffeeSupplyStateMachine);
	coffeePowderDispenser = createActivity(getCoffeePowderDispenser(), messageQueue_blocking);
}

static void runCoffeeSupply(void *activityarg) {
	logInfo("[coffeeSupply] Running...");

	while (TRUE) {
		// Wait for incoming message or time event
		CoffeeSupplyMessage incomingMessage;
		int result = waitForEvent(coffeeSupply, (char *)&incomingMessage, sizeof(incomingMessage), 1000);
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
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_init);
				break;
			case OFF_COMMAND:
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_switchOff);
				break;
			case SUPPLY_START_COMMAND:
				if (lastHasBeans) {
					processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_startSupplying);
				}
				break;
			case SUPPLY_STOP_COMMAND:
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_stop);
				break;
			case SUPPLY_NO_BEANS_ERROR:
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_noBeans);
				// notifiy mainController:
				sendMessage(getMainControllerDescriptor(), (char *)&(CoffeeSupplyMessage) {
					.activity = getCoffeeSupplyDescriptor(),
					.intValue = SUPPLY_NO_BEANS_ERROR,
					.strValue = "no beans",
				}, sizeof(MainControllerMessage), messagePriority_high);
				lastHasBeans = FALSE;
				break;
			case SUPPLY_BEANS_AVAILABLE_NOTIFICATION:
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_beansAvailable);
				sendMessage(getMainControllerDescriptor(), (char *)&(CoffeeSupplyMessage) {
					.activity = getCoffeeSupplyDescriptor(),
					.intValue = SUPPLY_BEANS_AVAILABLE_NOTIFICATION,
					.strValue = "beans available",
				}, sizeof(MainControllerMessage), messagePriority_high);
				lastHasBeans = TRUE;
				break;
			case OK_RESULT:
				MESSAGE_SELECTOR_BEGIN
				MESSAGE_SELECTOR(incomingMessage, coffeePowderDispenser)
					processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_supplyingFinished);
				MESSAGE_SELECTOR_END
				break;
			}
		}
		// Run state machine
		runStateMachine(&coffeeSupplyStateMachine);
		int hasCoffeeWasteState = hasCoffeeWaste();

		// Something happened with the waste bin?
		if (hasCoffeeWasteState != lastHasCoffeeWasteState ) {
			lastHasCoffeeWasteState = hasCoffeeWasteState;
			if (hasCoffeeWasteState) {
				sendMessage(getMainControllerDescriptor(),(char *)&(CoffeeSupplyMessage){
					.activity = getCoffeeSupplyDescriptor(),
					.intValue = WASTE_BIN_FULL_ERROR,
					.strValue = "Waste bin full"
					}, sizeof(MainControllerMessage), messagePriority_medium);
			} else {
				sendMessage(getMainControllerDescriptor(),(char *)&(CoffeeSupplyMessage){
					.activity = getCoffeeSupplyDescriptor(),
					.intValue = WASTE_BIN_CLEAR_NOTIFICATION,
					.strValue = "Waste bin clear"
					}, sizeof(MainControllerMessage), messagePriority_medium);
			}
		}
	}
}

static void tearDownCoffeeSupply(void *activity) {
	logInfo("[coffee supply] Tearing down...");
	destroyActivity(coffeePowderDispenser);
}

