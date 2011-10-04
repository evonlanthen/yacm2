/**
 * @brief   Submodule coffee powder dispenser
 * @file    coffeePowderDispenser.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "log.h"
#include "device.h"
#include "unistd.h"
#include "mainController.h"
#include "coffeePowderDispenser.h"
#include "coffeeSupply.h"
#include "activity.h"
#include "stateMachineEngine.h"

#define POWER_MAX 99
#define FLASH_MAX 255
#define POTENTIOMETER_MAX 1011

typedef enum {
	dispenseResult_ok,
	dispenseResult_nok
} DispenseResult;

static void setUpCoffeePowderDispenser(void *activity);
static void runCoffeePowderDispenser(void *activity);
static void tearDownCoffeePowderDispenser(void *activity);
static void setUpFillStateMonitor(void *activity);
static void runFillStateMonitor(void *activity);
static void tearDownFillStateMonitor(void *activity);
static void setUpMotorController(void *activity);
static void runMotorController(void *activity);
static void tearDownMotorController(void *activity);

static StateMachine coffeePowderDispenserStateMachine;
static int currentMotorPower = 0;

static Activity *coffeePowderDispenser;
static Activity *fillStateMonitor;
static Activity *motorController;

static ActivityDescriptor coffeePowderDispenserDescriptor = {
	.name = "coffeePowderDispenser",
	.setUp = setUpCoffeePowderDispenser,
	.run = runCoffeePowderDispenser,
	.tearDown = tearDownCoffeePowderDispenser
};

static ActivityDescriptor fillStateMonitorDescriptor = {
	.name = "coffeeBeansFillStateMonitor",
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
	if (power > POWER_MAX) {
		power = 99;
	} else if (power < 0) {
		power = 0;
	}

	int currentPower = readNonBlockingDevice("/dev/coffeeGrinderMotor");

	if (power != currentPower) {
		char powerAsString[3];
		snprintf(powerAsString, 3, "%d", power);
		writeNonBlockingDevice("/dev/coffeeGrinderMotor", powerAsString, wrm_replace, FALSE);
		/*
		 * adjust flash value as well, calculate it from power:
		 * power=0 => flash=200
		 * power=99 => flash=101
		 * TODO: the steps might be to big, adjust step!
		 */
		int step = 100;
		int flash = (FLASH_MAX - 55) /* set max value to 200 */ - (power * step / 100);
		char flashAsString[4];
		snprintf(flashAsString, 4, "%d", flash);
		//writeNonBlockingDevice("/dev/coffeeGrinderSPI", flashAsString, wrm_replace, FALSE);
	}

	return currentPower;
}

static int setMotorPotentiometerControlled() {
	int potentiometerValue;
	int power;

	// read value from potentiometer:
	potentiometerValue = readNonBlockingDevice("/proc/adc/ADC0");
	// calculate power:
	power = POWER_MAX * potentiometerValue / POTENTIOMETER_MAX;
	// set motor:
	int previousPower = setMotor(power);

	return previousPower;
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
		//logInfo("[fillStateMonitor] Beans state changed to %d",hasBeansState);
		lastHasBeansState = hasBeansState;

		if (hasBeansState) {
			sendMessage(getCoffeePowderDispenser(), (char *)&(FillStateMonitorMessage) {
				.activity = getCoffeeBeansFillStateMonitor(),
				.intValue = POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION,
			}, sizeof(FillStateMonitorMessage), messagePriority_high);
		} else {
			sendMessage(getCoffeePowderDispenser(), (char *)&(FillStateMonitorMessage) {
				.activity = getCoffeeBeansFillStateMonitor(),
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

/**
 * Represents a coffeePowderDispenser event
 */
typedef enum {
	coffeePowderDispenserEvent_init,
	coffeePowderDispenserEvent_switchOff,
	coffeePowderDispenserEvent_initialized,
	coffeePowderDispenserEvent_startSupplying,
	coffeePowderDispenserEvent_supplyingFinished,
	coffeePowderDispenserEvent_stop,
	coffeePowderDispenserEvent_noBeans,
	coffeePowderDispenserEvent_beansAvailable,
} CoffeePowderDispenserEvent;

/*
 ***************************************************************************
 * switchedOff state powder dispenser
 ***************************************************************************
 */

static void coffeePowderDispenserSwitchedOffStateEntryAction() {
	//logInfo("[coffeePowderDispenser] Entered SwitchedOff State...");
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
	// notifiy motorController:
	sendMessage(getCoffeeBeansFillStateMonitor(), (char *)&(FillStateMonitorMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = INIT_COMMAND,
		.strValue = "Init",
	}, sizeof(FillStateMonitorMessage), messagePriority_medium);
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
	//logInfo("[coffeePowderDispenser] Entered Idle State...");
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

static DispenseResult dispenseResult;
static int dispenseError;

static void coffeePowderDispenserSupplyingStateEntryAction() {
	//logInfo("[coffeePowderDispenser] Entered Supplying State...");
	dispenseResult = dispenseResult_nok;
	dispenseError = NO_ERROR;

	// notifiy motorController:
	sendMessage(getMotorController(), (char *)&(MotorControllerMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = MOTOR_START_COMMAND,
		.strValue = "start motor",
	}, sizeof(MotorControllerMessage), messagePriority_medium);
}

static Event coffeePowderDispenserSupplyingStateDoAction() {
	// enough Powder
	if (hasEnoughPowder()) {
		logInfo("[coffeePowderDispenser] Enough powder!");
		dispenseResult = dispenseResult_ok;
		return coffeePowderDispenserEvent_supplyingFinished;
	}
	// adjust motor power according to the current value of the potentiometer:
	setMotorPotentiometerControlled();

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
	sendMessage(getCoffeeSupplyDescriptor(), (char *)&(SimpleCoffeeSupplyMessage) {
		.activity = getCoffeePowderDispenser(),
		.intValue = dispenseResult == dispenseResult_ok ? OK_RESULT : NOK_RESULT,
		.strValue = "grinding complete",
	}, sizeof(SimpleCoffeeSupplyMessage), messagePriority_medium);
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
	.name = "coffeePowderDispenser",
	.numberOfEvents = 8,
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

ActivityDescriptor getCoffeePowderDispenser() {
	return coffeePowderDispenserDescriptor;
}

ActivityDescriptor getCoffeeBeansFillStateMonitor() {
	return fillStateMonitorDescriptor;
}

ActivityDescriptor getMotorController() {
	return motorControllerDescriptor;
}

static void setUpCoffeePowderDispenser(void *activityarg) {
	//logInfo("[coffeePowderDispenser] Setting up...");
	coffeePowderDispenser = activityarg;
	setUpStateMachine(&coffeePowderDispenserStateMachine);
	if (!coffeePowderDispenserStateMachine.isInitialized) {
		logErr("[coffeePowderDispenser] Statemachine init failed!");
	}
	createActivity(getCoffeeBeansFillStateMonitor(), messageQueue_blocking);
	createActivity(getMotorController(), messageQueue_blocking);
}

static void runCoffeePowderDispenser(void *activity) {
	//logInfo("[coffeePowderDispenser] Running...");
	while (TRUE) {
		// Wait for incoming message or time event
		CoffeePowderDispenserMessage incomingMessage;
		int result = waitForEvent(coffeePowderDispenser, (char *)&incomingMessage, sizeof(incomingMessage), 100);
		if (result < 0) {
			//TODO Implement appropriate error handling
			sleep(10);
			// Try to recover from error
			continue;
		}

		// Check if there is an incoming message
		if (result > 0) {
			// Process incoming message
			//logInfo("[coffeePowderDispenser] Processing incoming message...");
			switch (incomingMessage.intValue) {
				case INIT_COMMAND:
					//logInfo("[coffeePowderDispenser] Received init command...");
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_init);
					break;
				case OFF_COMMAND:
					//logInfo("[coffeePowderDispenser] Received off command...");
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_switchOff);
					break;
				case POWDER_DISPENSER_START_COMMAND:
					//logInfo("[coffeePowderDispenser] Received start command...");
					if (lastHasBeansState) {
						processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_startSupplying);
					}
					break;
				case POWDER_DISPENSER_STOP_COMMAND:
					//logInfo("[coffeePowderDispenser] Received stop command...");
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_stop);
					break;
				case POWDER_DISPENSER_NO_BEANS_ERROR:
					//logInfo("[coffeePowderDispenser] Received no beans error...");
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_noBeans);
					dispenseError = SUPPLY_NO_BEANS_ERROR;
					sendMessage(getCoffeeSupplyDescriptor(),(char *)&(SimpleCoffeeSupplyMessage){
						.activity = getCoffeePowderDispenser(),
						.intValue = SUPPLY_NO_BEANS_ERROR,
						.strValue = "No beans"
						}, sizeof(SimpleCoffeeSupplyMessage), messagePriority_medium);
					break;
				case POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION:
					//logInfo("[coffeePowderDispenser] Received beans available notification...");
					processStateMachineEvent(&coffeePowderDispenserStateMachine, coffeePowderDispenserEvent_beansAvailable);
					sendMessage(getCoffeeSupplyDescriptor(),(char *)&(SimpleCoffeeSupplyMessage){
						.activity = getCoffeePowderDispenser(),
						.intValue = SUPPLY_BEANS_AVAILABLE_NOTIFICATION,
						.strValue = "Beans available"
						}, sizeof(SimpleCoffeeSupplyMessage), messagePriority_medium);
					break;
			}
		}
		// Run state machine
		runStateMachine(&coffeePowderDispenserStateMachine);
	}
}

static void tearDownCoffeePowderDispenser(void *activity) {
	//logInfo("[coffeePowderDispenser] Tearing down...");
	destroyActivity(fillStateMonitor);
	destroyActivity(motorController);
}

static void setUpFillStateMonitor(void *activityarg) {
	//logInfo("[fillStateMonitor] Setting up...");
	fillStateMonitor = activityarg;
	lastHasBeansState = hasBeans();
	if (lastHasBeansState) {
		//logInfo("[fillStateMonitor] Init: sending Beans available");
		sendMessage(getCoffeePowderDispenser(),(char *)&(SimpleCoffeeSupplyMessage){
			.activity = getCoffeeBeansFillStateMonitor(),
			.intValue = POWDER_DISPENSER_BEANS_AVAILABLE_NOTIFICATION,
			.strValue = "Beans available"
			}, sizeof(CoffeePowderDispenserMessage), messagePriority_medium);
	} else {
		//logInfo("[fillStateMonitor] Init: sending no beans error");
		sendMessage(getCoffeePowderDispenser(),(char *)&(SimpleCoffeeSupplyMessage){
			.activity = getCoffeeBeansFillStateMonitor(),
			.intValue = POWDER_DISPENSER_NO_BEANS_ERROR,
			.strValue = "No beans"
			}, sizeof(CoffeePowderDispenserMessage), messagePriority_medium);
	}
}

static void runFillStateMonitor(void *activity) {
	//logInfo("[fillStateMonitor] Running...");

	while (TRUE) {
		// Wait for incoming message or time event
		FillStateMonitorMessage incomingMessage;
		int result = waitForEvent(fillStateMonitor, (char *)&incomingMessage, sizeof(incomingMessage), 100);
		if (result < 0) {
			//TODO Implement appropriate error handling
			sleep(10);
				// Try to recover from error
			continue;
		}

		// Check if there is an incoming message
		if (result > 0) {
			// Process incoming message
			//logInfo("[fillStateMonitor] Process incoming message...");
		}
		checkBeans();
	}
}

static void tearDownFillStateMonitor(void *activity) {
	//logInfo("[FillStateMonitor] Tearing down...");
}

static void setUpMotorController(void *activityarg) {
	//logInfo("[motorController] Setting up...");
	motorController = activityarg;
	//setMotor(0);
}

static void runMotorController(void *activity) {
	//logInfo("[motorController] Running...");

	int previousMotorPower = -1;

	while (TRUE) {
		//logInfo("[motorController] Going to receive message...");
		MotorControllerMessage message;
		receiveMessage(activity, (char *)&message, sizeof(message));
		//logInfo("[motorController] Message received from %s (length: %ld): value: %d, message: %s", message.activity.name, messageLength, message.intValue, message.strValue);
		switch (message.intValue) {
			case MOTOR_START_COMMAND:
				currentMotorPower = 0;
				//setMotor(50);
				previousMotorPower = setMotorPotentiometerControlled();
				break;
			case MOTOR_STOP_COMMAND:
				//setMotor(0);
				if (previousMotorPower != -1) {
					setMotor(previousMotorPower);
				}
				break;
		}
	}
}

static void tearDownMotorController(void *activity) {
	//logInfo("[motorController] Tearing down...");
	//setMotor(0);
}
