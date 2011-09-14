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
ActivityDescriptor getFillStateMonitor();
ActivityDescriptor getMotorController();

static StateMachine coffeeSupplyStateMachine;
static StateMachine coffeePowderDispenserStateMachine;

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

static int setMotor(int power) {
	char level[2];
	if (power > 99) power = 99;
	if (power < 0) power = 0;
	snprintf(level,2,"%d",power);
	return writeNonBlockingDevice("/dev/coffeeGrinderMotor",level,wrm_replace,FALSE);
}

static int ejectWaste(void) {
	return writeNonBlockingDevice("/dev/coffeeWasteEjector","1",wrm_replace,FALSE);
}

static int lastHasCoffeeWasteState = TRUE;

static int hasCoffeeWaste(void) {
	return readNonBlockingDevice("./dev/coffeeWasteSensor");
}

static int hasEnoughPowder(void) {
	return readNonBlockingDevice("./dev/coffeePowderDispenser");
}

static int hasBeans(void) {
	return readNonBlockingDevice("./dev/coffeeBeansSensor");
}

static int lastHasBeansState = FALSE;

static int checkBeans(void) {
	int hasBeansState = hasBeans();
	if (hasBeansState != lastHasBeansState) {
		logInfo("[fillStateMonitor] Beans state changed to %d",hasBeansState);
		lastHasBeansState = hasBeansState;

		if (hasBeansState) {
			sendMessage(getCoffeePowderDispenser(), (char *)&(FillStateMonitorMessage) {
				.activity = getFillStateMonitor(),
				.intValue = POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION,
			}, sizeof(FillStateMonitorMessage), messagePriority_high);
		} else {
			sendMessage(getCoffeePowderDispenser(), (char *)&(FillStateMonitorMessage) {
				.activity = getFillStateMonitor(),
				.intValue = POWDER_DISPENSER_NO_BEANS_ERROR,
			}, sizeof(FillStateMonitorMessage), messagePriority_high);
		}
	}

	return hasBeansState;
}

/*
 ***************************************************************************
 * States coffeePowderDispenser
 ***************************************************************************
 */

// off:
// initializing:
// idle: bereit;
// supplying;
/**
 * Represents a coffee powder dispenser state
 */
typedef enum {
	coffeePowderDispenserState_switchedOff,
	coffeePowderDispenserState_initializing,
	coffeePowderDispenserState_idle,
	coffeePowderDispenserState_supplying
} CoffeePowderDispenserState;

/*
 ***************************************************************************
 * switchedOff state powder dispenser
 ***************************************************************************
 */

static void coffeePowderDispenserSwitchedOffStateEntryAction() {
	logInfo("[coffeePowderDispenser] Entered SwitchedOff State...");
}

static Event coffeePowderDispenserSwitchedOffStateDoAction() {
	return NO_EVENT;
}

static State coffeePowderDispenserSwitchedOffState = {
	.stateIndex = coffeePowderDispenserState_switchedOff,
	.entryAction = coffeePowderDispenserSwitchedOffStateEntryAction,
	.doAction = coffeePowderDispenserSwitchedOffStateDoAction
};


/*
 ***************************************************************************
 * initializing state powder dispenser
 ***************************************************************************
 */

static void coffeePowderDispenserInitializingStateEntryAction() {
	// notifiy motorController:
	sendMessage(getMotorController(), (char *)&(MotorControllerMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = MOTOR_STOP_COMMAND,
		.strValue = "stop motor",
	}, sizeof(MotorControllerMessage), messagePriority_medium);
}

static Event coffeePowderDispenserInitializingStateDoAction() {
	sleep(1);
	return coffeePowderDispenserEvent_initialized;
}


static State coffeePowderDispenserInitializingState = {
	.stateIndex = coffeePowderDispenserState_initializing,
	.entryAction = coffeePowderDispenserInitializingStateEntryAction,
	.doAction = coffeePowderDispenserInitializingStateDoAction
};


/*
 ***************************************************************************
 * idle state powder dispenser
 ***************************************************************************
 */

static void coffeePowderDispenserIdleStateEntryAction() {
	;
}

static Event coffeePowderDispenserIdleStateDoAction() {

	return NO_EVENT;
}

static State coffeePowderDispenserIdleState = {
	.stateIndex = coffeePowderDispenserState_idle,
	.entryAction = coffeePowderDispenserIdleStateEntryAction,
	.doAction = coffeePowderDispenserIdleStateDoAction
};

/*
 ***************************************************************************
 * supplying state powder dispenser
 ***************************************************************************
 */

static void coffeePowderDispenserSupplyingStateEntryAction() {
	// notifiy motorController:
	sendMessage(getMotorController(), (char *)&(MotorControllerMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = MOTOR_START_COMMAND,
		.strValue = "start motor",
	}, sizeof(MotorControllerMessage), messagePriority_medium);
}

static Event coffeePowderDispenserSupplyingStateDoAction() {
	return NO_EVENT;
}

static void coffeePowderDispenserSupplyingStateExitAction() {
	// notifiy motorController:
	sendMessage(getMotorController(), (char *)&(MotorControllerMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = MOTOR_STOP_COMMAND,
		.strValue = "stop motor",
	}, sizeof(MotorControllerMessage), messagePriority_medium);
	// notifiy coffeeSupply:
	sendMessage(getCoffeeSupplyDescriptor(), (char *)&(CoffeeSupplyMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = OK_RESULT,
		.strValue = "grinding complete",
	}, sizeof(CoffeeSupplyMessage), messagePriority_medium);
}

static State coffeePowderDispenserSupplyingState = {
	.stateIndex = coffeePowderDispenserState_supplying,
	.entryAction = coffeePowderDispenserSupplyingStateEntryAction,
	.doAction = coffeePowderDispenserSupplyingStateDoAction,
	.exitAction = coffeePowderDispenserSupplyingStateExitAction
};


/*
 ***************************************************************************
 * State transitions powder dispenser
 ***************************************************************************
 */

static StateMachine coffeePowderDispenserStateMachine = {
	.numberOfEvents = 7,
	.initialState = &coffeePowderDispenserSwitchedOffState,
	.transitions = {
		/* coffeePowderDispenserState_switchedOff: */
			/* coffeePowderDispenserEvent_init: */ &coffeePowderDispenserInitializingState,
			/* coffeePowderDispenserEvent_switchOff: */ NULL,
			/* coffeePowderDispenserEvent_initialized: */ NULL,
			/* coffeePowderDispenservent_startSupplying: */ NULL,
			/* coffeePowderDispenserEvent_supplyingFinished: */ NULL,
			/* coffeePowderDispenserEvent_stop: */ NULL,
			/* coffeePowderDispenserEvent_noBeans: */ NULL,
			/* coffeePowderDispenserEvent_beansAvailable: */ NULL,
		/* coffeePowderDispenserState_initializing: */
			/* coffeePowderDispenserEvent_init: */ NULL,
			/* coffeePowderDispenserEvent_switchOff: */ &coffeePowderDispenserSwitchedOffState,
			/* coffeePowderDispenserEvent_initialized: */ &coffeePowderDispenserIdleState,
			/* coffeePowderDispenserEvent_startSupplying: */ NULL,
			/* coffeePowderDispenserEvent_supplyingFinished: */ NULL,
			/* coffeePowderDispenserEvent_stop: */ NULL,
			/* coffeePowderDispenserEvent_noBeans: */ NULL,
			/* coffeePowderDispenserEvent_beansAvailable: */ NULL,
		/* coffeePowderDispenserState_idle: */
			/* coffeePowderDispenserEvent_init: */ &coffeePowderDispenserInitializingState,
			/* coffeePowderDispenserEvent_switchOff: */ &coffeePowderDispenserSwitchedOffState,
			/* coffeePowderDispenserEvent_initialized: */ NULL,
			/* coffeePowderDispenserEvent_startSupplying: */ &coffeePowderDispenserSupplyingState,
			/* coffeePowderDispenserEvent_supplyingFinished: */ NULL,
			/* coffeePowderDispenserEvent_stop: */ NULL,
			/* coffeePowderDispenserEvent_noBeans: */ NULL,
			/* coffeePowderDispenserEvent_beansAvailable: */ NULL,
		/* coffeePowderDispenserState_supplying: */
			/* coffeePowderDispenserEvent_init: */ NULL,
			/* coffeePowderDispenserEvent_switchOff: */ &coffeePowderDispenserSwitchedOffState,
			/* coffeePowderDispenserEvent_initialized: */ NULL,
			/* coffeePowderDispenserEvent_startSupplying: */ NULL,
			/* coffeePowderDispenserEvent_supplyingFinished: */ &coffeePowderDispenserIdleState,
			/* coffeePowderDispenserEvent_stop: */ &coffeePowderDispenserIdleState,
			/* coffeePowderDispenserEvent_noBeans: */ &coffeePowderDispenserIdleState,
			/* coffeePowderDispenserEvent_beansAvailable: */ NULL
		}
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
	//TODO: Send correct init message to powder dispenser
	sendMessage(getCoffeePowderDispenser(), (char *)&(CoffeePowderDispenserMessage){
			.activity = getCoffeeSupplyDescriptor(),
			.intValue = 123,
			.strValue = "abc"
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
	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
		.activity = getCoffeeSupplyDescriptor(),
		.intValue = 1,
		.strValue = "coffee powder supplying started",
	}, sizeof(MainControllerMessage), messagePriority_low);
}

static Event coffeeSupplySupplyingStateDoAction() {
	return NO_EVENT;
}

static void coffeeSupplySupplyingStateExitAction() {
	// eject Waste
	ejectWaste();
	// notifiy mainController:
	sendMessage(getMainControllerDescriptor(), (char *)&(MainControllerMessage) {
		.activity = getCoffeeSupplyDescriptor(),
		.intValue = OK_RESULT,
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
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_startSupplying);
				break;
			case SUPPLY_STOP_COMMAND:
				processStateMachineEvent(&coffeeSupplyStateMachine, coffeeSupplyEvent_stop);
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
				sendMessage(getMainControllerDescriptor(),(char *)&(MainControllerMessage){
					.activity = getCoffeeSupplyDescriptor(),
					.intValue = WASTE_BIN_FULL_ERROR,
					.strValue = "Waste bin full"
					}, sizeof(MainControllerMessage), messagePriority_medium);
			} else {
				sendMessage(getMainControllerDescriptor(),(char *)&(MainControllerMessage){
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

static void setUpCoffeePowderDispenser(void *activityarg) {
	logInfo("[coffeePowderDispenser] Setting up...");
	coffeePowderDispenser = activityarg;
	setUpStateMachine(&coffeePowderDispenserStateMachine);
	fillStateMonitor = createActivity(getFillStateMonitor(), messageQueue_blocking);
	motorController = createActivity(getMotorController(), messageQueue_blocking);
}

static void runCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Running...");
	while (TRUE) {
		// Wait for incoming message or time event
		CoffeePowderDispenserMessage incomingMessage;
		int result = waitForEvent(coffeePowderDispenser, (char *)&incomingMessage, sizeof(incomingMessage), 1000);
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
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_init);
					break;
				case OFF_COMMAND:
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_switchOff);
					break;
				case POWDER_DISPENSER_START_COMMAND:
					if (lastHasBeansState) {
						processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_startSupplying);
					}
					break;
				case POWDER_DISPENSER_STOP_COMMAND:
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_stop);
					break;
				case POWDER_DISPENSER_NO_BEANS_ERROR:
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_noBeans);
					//TODO: Send message to coffeeSupply
					break;
				case POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION:
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_beansAvailable);
					//TODO: Send message to coffeeSupply
					break;
			}
		}
	}
	// Run state machine
	runStateMachine(&coffeePowderDispenserStateMachine);
	// enough Powder
	if (hasEnoughPowder()) {
		processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_supplyingFinished);
	}
}

static void tearDownCoffeePowderDispenser(void *activity) {
	logInfo("[coffeePowderDispenser] Tearing down...");
	destroyActivity(fillStateMonitor);
	destroyActivity(motorController);
}

static void setUpFillStateMonitor(void *activity) {
	logInfo("[fillStateMonitor] Setting up...");
	lastHasBeansState = hasBeans();
}

static void runFillStateMonitor(void *activity) {
	logInfo("[fillStateMonitor] Running...");

	while (TRUE) {
		checkBeans();
		usleep(100);
	}
}

static void tearDownFillStateMonitor(void *activity) {
	logInfo("[coffeePowderDispenser] Tearing down...");
}

static void setUpMotorController(void *activity) {
	logInfo("[motorController] Setting up...");
	setMotor(0);
}

static void runMotorController(void *activity) {
	logInfo("[motorController] Running...");

	while (TRUE) {

		logInfo("[motorController] Going to receive message...");
		MotorControllerMessage message;
		unsigned long messageLength = receiveMessage(activity, (char *)&message, sizeof(message));
		logInfo("[motorController] Message received from %s (length: %ld): value: %d, message: %s",
				message.activity.name, messageLength, message.intValue, message.strValue);
		switch (message.intValue) {
			case MOTOR_START_COMMAND:
				setMotor(50);
				break;
			case MOTOR_STOP_COMMAND:
				setMotor(0);
				break;
		}
	}
}

static void tearDownMotorController(void *activity) {
	logInfo("[motorController] Tearing down...");
	setMotor(0);
}
