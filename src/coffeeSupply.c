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
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "mainController.h"
#include "coffeeSupply.h"
#include "activity.h"
#include "stateMachineEngine.h"

static void setUpCoffeeSupply(void *activity);
static void runCoffeeSupply(void *activity);
static void tearDownCoffeeSupply(void *activity);
static void setUpCoffeePowderDispenser(void *activity);
static void runCoffeePowderDispenser(void *activity);
static void tearDownCoffeePowderDispenser(void *activity);
static void setUpFillStateMonitor(void *activity);
static void runFillStateMonitor(void *activity);
static void tearDownFillStateMonitor(void *activity);
static void setUpMotorController(void *activity);
static void runMotorController(void *activity);
static void tearDownMotorController(void *activity);
ActivityDescriptor getCoffeePowderDispenser();

static StateMachine coffeeSupplyStateMachine;
static StateMachine coffeePowderDispenserStateMachine;
static StateMachine FillStateMonitorStateMachine;
static StateMachine MotorControllerStateMachine;

static Activity *coffeePowderDispenser;
static Activity *fillStateMonitor;
static Activity *motorController;
static Activity *coffeeSupply;

static ActivityDescriptor coffeeSupplyDescriptor = {
	.name = "coffeeSupply",
	.setUp = setUpCoffeeSupply,
	.run = runCoffeeSupply,
	.tearDown = tearDownCoffeeSupply
};

static ActivityDescriptor coffeePowderDispenserDescriptor = {
	.name = "coffeePowderDispenser",
	.setUp = setUpCoffeePowderDispenser,
	.run = runCoffeePowderDispenser,
	.tearDown = tearDownCoffeePowderDispenser
};

static ActivityDescriptor fillStateMonitorDescriptor = {
	.name = "fillStateMonitor",
	.setUp = setUpFillStateMonitor,
	.run = runFillStateMonitor,
	.tearDown = tearDownFillStateMonitor
};

static ActivityDescriptor motorControllerDescriptor = {
	.name = "motorController",
	.setUp = setUpMotorController,
	.run = runMotorController,
	.tearDown = tearDownMotorController
};

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
	logInfo("[coffeeSupply] Going to receive message...");
	CoffeeSupplyMessage message;
	unsigned long messageLength = receiveMessage(coffeeSupply, (char *)&message, sizeof(message));
	if (messageLength > 0) {
		logInfo("[coffeeSupply] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
		return message.intValue;
	}
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
	sendMessage(getCoffeePowderDispenser(), (char *)&(CoffeePowderDispenserMessage){
			.activity = getCoffeeSupplyDescriptor(),
			.intValue = 123,
			.strValue = "abc"
			}, sizeof(CoffeePowderDispenserMessage), messagePriority_medium);
	logInfo("[coffeeSupply] ...done. (send message)");
}

static void coffeeSupplyInitializingStateDoAction() {
	;
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
	CoffeeSupplyMessage incomingMessage;
	unsigned long incomingMessageLength;
	logInfo("[coffeeSupply] IdleState: Going to receive message...");
	incomingMessageLength = receiveMessage(coffeeSupply, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[coffeeSupply] IdleState: Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
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
	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
		.activity = getCoffeeSupplyDescriptor(),
		.intValue = 1,
		.strValue = "coffee powder supplying started",
	}, sizeof(MainControllerMessage), messagePriority_low);
}

static Event coffeeSupplySupplyingStateDoAction() {
	unsigned long incomingMessageLength;
	CoffeeSupplyMessage incomingMessage;
	logInfo("[coffeeSupply] Going to receive message...");
	incomingMessageLength = receiveMessage(coffeeSupply, (char *)&incomingMessage, sizeof(incomingMessage));
	if (incomingMessageLength > 0) {
		logInfo("[coffeeSupply] SupplyState: Message received from %s (length: %d): value: %d, message: %s",
				incomingMessage.activity.name, incomingMessageLength, incomingMessage.intValue, incomingMessage.strValue);
		return incomingMessage.intValue;
	}
	return NO_EVENT;
}

static void coffeeSupplySupplyingStateExitAction() {

	// notifiy mainController:
	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
		.activity = getCoffeeSupplyDescriptor(),
		.intValue = 1,
		.strValue = "coffee powder supplying finished",
	}, sizeof(MainControllerMessage), messagePriority_high);
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
		/* coffeeSupplyState_initializing: */
			/* coffeeSupplyEvent_init: */ NULL,
			/* coffeeSupplyEvent_switchOff: */ &coffeeSupplySwitchedOffState,
			/* coffeeSupplyEvent_initialized: */ &coffeeSupplyIdleState,
			/* coffeeSupplyEvent_startSupplying: */ NULL,
			/* coffeeSupplyEvent_supplyingFinished: */ NULL,
			/* coffeeSupplyEvent_stop: */ NULL,
		/* coffeeSupplyState_idle: */
			/* coffeeSupplyEvent_init: */ &coffeeSupplyInitializingState,
			/* coffeeSupplyEvent_switchOff: */ &coffeeSupplySwitchedOffState,
			/* coffeeSupplyEvent_initialized: */ NULL,
			/* coffeeSupplyEvent_startSupplying: */ &coffeeSupplySupplyingState,
			/* coffeeSupplyEvent_supplyingFinished: */ NULL,
			/* coffeeSupplyEvent_stop: */ NULL,
		/* coffeeSupplyState_supplying: */
			/* coffeeSupplyEvent_init: */ NULL,
			/* coffeeSupplyEvent_switchOff: */ &coffeeSupplySwitchedOffState,
			/* coffeeSupplyEvent_initialized: */ NULL,
			/* coffeeSupplyEvent_startSupplying: */ NULL,
			/* coffeeSupplyEvent_supplyingFinished: */ &coffeeSupplyIdleState,
			/* coffeeSupplyEvent_stop: */ &coffeeSupplyIdleState
		}
};

ActivityDescriptor getCoffeeSupplyDescriptor() {
	return coffeeSupplyDescriptor;
}

ActivityDescriptor getCoffeePowderDispenser() {
	return coffeePowderDispenserDescriptor;
}

ActivityDescriptor getFillStateMonitor() {
	return fillStateMonitorDescriptor;
}

ActivityDescriptor getMotorController() {
	return motorControllerDescriptor;
}

static void setUpCoffeeSupply(void *activityarg) {
	logInfo("[coffeeSupply] Setting up...");
	coffeePowderDispenser = createActivity(getCoffeePowderDispenser(), messageQueue_blocking);
}

static void runCoffeeSupply(void *activityarg) {
	logInfo("[coffeeSupply] Running...");
	coffeeSupply = activityarg;
	setUpStateMachine(&coffeeSupplyStateMachine);

	while (TRUE) {
		// Run state machine
		runStateMachine(&coffeeSupplyStateMachine);
	}
}

static void tearDownCoffeeSupply(void *activity) {
	logInfo("[coffee supply] Tearing down...");
}

static void setUpCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Setting up...");
	fillStateMonitor = createActivity(getFillStateMonitor(), messageQueue_blocking);
	motorController = createActivity(getMotorController(), messageQueue_blocking);
}

static void runCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Running...");

	while (TRUE) {
		logInfo("[coffeePowderDispenser] Going to receive message...");
		CoffeePowderDispenserMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[coffeePowderDispenser] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Tearing down...");
}

static void setUpFillStateMonitor(void *activity) {
	logInfo("[fillStateMonitor] Setting up...");
}

static void runFillStateMonitor(void *activity) {
	logInfo("[fillStateMonitor] Running...");

	while (TRUE) {
		logInfo("[fillStateMonitor] Going to receive message...");
		FillStateMonitorMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[fillStateMonitor] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownFillStateMonitor(void *activity) {
	logInfo("[coffeePowderDispenser] Tearing down...");
}

static void setUpMotorController(void *activity) {
	logInfo("[motorController] Setting up...");
}

static void runMotorController(void *activity) {
	logInfo("[motorController] Running...");

	while (TRUE) {
		logInfo("[motorController] Going to receive message...");
		MotorControllerMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[motorController] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
	}
}

static void tearDownMotorController(void *activity) {
	logInfo("[motorController] Tearing down...");
}
