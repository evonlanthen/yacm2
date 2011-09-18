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

MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply, GrindCoffeePowderCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply, EjectCoffeeWasteCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply, Result, 3)
MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply, BeanStatus, 4)
MESSAGE_CONTENT_TYPE_MAPPING(CoffeeSupply, WasteBinStatus, 5)

static int ejectWaste(void) {
	return writeNonBlockingDevice("./dev/coffeeWasteEjector","1",wrm_replace,FALSE);
}

static int lastHasCoffeeWasteState = TRUE;

static int hasCoffeeWaste(void) {
	return readNonBlockingDevice("./dev/coffeeWasteSensor");
}

// coffeeSupply state for beans
static Availability lastHasBeans = notAvailable;

// should waste be ejected
static int wasteDisposable = FALSE;


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
	//logInfo("[coffeeSupply] Entered SwitchedOff State...");
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
	//logInfo("[coffeeSupply] Send Init message to coffeePowderDispenser...");
	//Send init message to powder dispenser
	sendMessage(getCoffeePowderDispenser(), (char *)&(CoffeePowderDispenserMessage){
			.activity = getCoffeeSupplyDescriptor(),
			.intValue = INIT_COMMAND,
			.strValue = "init coffeePowderDispenser"
			}, sizeof(CoffeePowderDispenserMessage), messagePriority_medium);
	//logInfo("[coffeeSupply] ...done. (send init message)");
}

static Event coffeeSupplyInitializingStateDoAction() {
	// Eject waste
	ejectWaste();
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
	//logInfo("[coffeeSupply] Entered Supplying State...");
	//Check if we should eject waste
	if (wasteDisposable) {
		ejectWaste();
		//logInfo("[coffeeSupply] Ejecting without notification from maincontroller");
		wasteDisposable = FALSE;
	}
	logInfo("[coffeeSupply] Going to grind coffee powder...");
	//Send init message to powder dispenser
	sendMessage(getCoffeePowderDispenser(), (char *)&(CoffeePowderDispenserMessage){
		.activity = getCoffeeSupplyDescriptor(),
		.intValue = POWDER_DISPENSER_START_COMMAND,
		.strValue = "start coffeePowderDispenser"
		}, sizeof(CoffeePowderDispenserMessage), messagePriority_medium);
	//logInfo("[coffeeSupply] ...done. (send dispenser start message)");
}

static Event coffeeSupplySupplyingStateDoAction() {
	return NO_EVENT;
}

static void coffeeSupplySupplyingStateExitAction() {
	wasteDisposable = TRUE;
	logInfo("[coffeeSupply] ...done (grinding coffee powder)");
	sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), Result)
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
	.name = "coffeeSupply",
	.numberOfEvents = 8,
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
	//logInfo("[coffeeSupply] Setting up...");
	coffeeSupply = activityarg;
	setUpStateMachine(&coffeeSupplyStateMachine);
	coffeePowderDispenser = createActivity(getCoffeePowderDispenser(), messageQueue_blocking);
}

static void runCoffeeSupply(void *activityarg) {
	//logInfo("[coffeeSupply] Running...");

	while (TRUE) {
		// Wait for incoming message or time event
		SimpleCoffeeSupplyMessage incomingMessage;
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
			//logInfo("[coffeeSupply] Process incoming message...");
			switch (incomingMessage.intValue) {
			case INIT_COMMAND:
				//logInfo("[coffeeSupply] Received init command");
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_init);
				break;
			case OFF_COMMAND:
				//logInfo("[coffeeSupply] Received off command");
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_switchOff);
				break;
			case SUPPLY_START_COMMAND:
				//logInfo("[coffeeSupply] Received supply start command");
				if (lastHasBeans == available) {
					//logInfo("[coffeeSupply] Beans available, starting supply");
					processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_startSupplying);
				} else {
					logInfo("[coffeeSupply] No beans!");
					sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), Result)
						.code = NOK_RESULT,
						.errorCode = NO_COFFEE_BEANS_ERROR
					sendNotification_END
				}
				break;
			case SUPPLY_STOP_COMMAND:
				//logInfo("[coffeeSupply] Received supply stop command");
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_stop);
				break;
			case SUPPLY_NO_BEANS_ERROR:
				//logInfo("[coffeeSupply] Received no beans error");
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_noBeans);
				// notifiy mainController:
				sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), BeanStatus)
					.availability = notAvailable
				sendNotification_END

				lastHasBeans = notAvailable;
				break;
			case SUPPLY_BEANS_AVAILABLE_NOTIFICATION:
				//logInfo("[coffeeSupply] Received beans available command");
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_beansAvailable);

				sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), BeanStatus)
					.availability = available
				sendNotification_END

				lastHasBeans = available;
				break;
			case EJECT_COFFEE_WASTE_COMMAND:
				//logInfo("[coffeeSupply] Received eject command");
				if (coffeeSupplyStateMachine.activeState == &coffeeSupplyIdleState) {
					ejectWaste();

					logInfo("[coffeeSupply] Coffee waste ejected");

					wasteDisposable = FALSE;

					sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), Result)
						.code = OK_RESULT
					sendNotification_END
				} else {
					sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), Result)
						.code = NOK_RESULT,
						.errorCode = COFFEE_WASTE_EJECTION_NOT_POSSIBLE_ERROR
					sendNotification_END
				}
				break;
			case OK_RESULT:
				//logInfo("[coffeeSupply] Received ok result");
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
			sendNotification_BEGIN(coffeeSupply, CoffeeSupply, getMainControllerDescriptor(), WasteBinStatus)
				.isBinFull = lastHasCoffeeWasteState
			sendNotification_END
		}
	}
}

static void tearDownCoffeeSupply(void *activity) {
	//logInfo("[coffee supply] Tearing down...");
	destroyActivity(coffeePowderDispenser);
}
