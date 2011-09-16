/**
 * @brief   Main system control module
 * @file    mainController.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "data.h"
#include "device.h"
#include "syslog.h"
#include "memoryManagement.h"
#include "stateMachineEngine.h"
#include "coffeeSupply.h"
#include "waterSupply.h"
#include "milkSupply.h"
//#include "userInterface.h"
#include "mainController.h"

static void setUpMainController(void *activity);
static void runMainController(void *activity);
static void tearDownMainController(void *activity);

static ActivityDescriptor mainControllerDescriptor = {
		.name = "mainController",
		.setUp = setUpMainController,
		.run = runMainController,
		.tearDown = tearDownMainController
};

MESSAGE_CONTENT_TYPE_MAPPING(MainController, InitCommand, 1)
MESSAGE_CONTENT_TYPE_MAPPING(MainController, OffCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(MainController, ProduceProductCommand, 3)
MESSAGE_CONTENT_TYPE_MAPPING(MainController, MachineStateChangedNotification, 4)
MESSAGE_CONTENT_TYPE_MAPPING(MainController, IngredientAvailabilityChangedNotification, 5)

static Activity *this;

static unsigned productToProduceIndex = 0;
static unsigned int produceWithMilk = FALSE;

static ActivityDescriptor clientDescriptor;

ActivityDescriptor getMainControllerDescriptor() {
	return mainControllerDescriptor;
}

/**
 * Represents an activity (= state) within the coffee making process.
 */
typedef enum {
	coffeeMakingActivity_warmingUp,
	coffeeMakingActivity_checkingCupFillState,
	coffeeMakingActivity_grindingCoffeePowder,
	coffeeMakingActivity_supplyingWater,
	coffeeMakingActivity_withMilkGateway,
	coffeeMakingActivity_supplyingMilk,
	coffeeMakingActivity_ejectingCoffeeWaste,
	coffeeMakingActivity_finished,
	coffeeMakingActivity_error,
	coffeeMakingActivity_undefined
} CoffeeMakingActivity;

/**
 * Represents an ongoing coffee making process instance.
 */
typedef struct {
	unsigned int productIndex; /**< The product currently produced. */
	int withMilk; /**< Is the product produced with milk? */
	CoffeeMakingActivity currentActivity; /**< The activity which is currently executed. */
} MakeCoffeeProcessInstance;

/**
 * Represents the coffee maker.
 */
typedef struct {
	MachineState state; /**< The coffee maker's state. */
	//Coffee coffee; /**< The coffee ingredient. */
	Availability isCoffeeAvailable;
	//Water water; /**< The water ingredient. */
	Availability isWaterAvailable;
	//Milk milk; /**< The milk ingredient. */
	Availability isMilkAvailable;
	//ProductListElement *products; /**< The product definition collection. */
	MakeCoffeeProcessInstance *ongoingCoffeeMaking; /**< A possibly ongoing coffee making process instance. */
} CoffeeMaker;

/**
 * The coffee maker model instance.
 */
static CoffeeMaker coffeeMaker = {
		.state = machineState_off,
		.isCoffeeAvailable = notAvailable,
		.isWaterAvailable = notAvailable,
		.isMilkAvailable = notAvailable,
};

// =============================================================================
// State machine definitions
// =============================================================================

// =============================================================================
// Main state machine
// =============================================================================

static StateMachine coffeeMakingProcessMachine;

// -----------------------------------------------------------------------------
// Events
// -----------------------------------------------------------------------------

/**
 * Represents a coffee maker event.
 */
typedef enum {
	event_switchedOn,
	event_switchedOff,
	event_isInitialized,
	event_productSelected,
	event_productionProcessAborted,
	event_productionProcessIsFinished,
	event_ingredientTankIsEmpty
} MainControllerEvent;

// -----------------------------------------------------------------------------
// Off state
// -----------------------------------------------------------------------------

static void offStateEntryAction() {
	// Switch off milk supply
	sendRequest_BEGIN(this, MilkSupply, OffCommand)
	sendRequest_END
	// Switch off water supply
	sendRequest_BEGIN(this, WaterSupply, OffCommand)
	sendRequest_END
	// Switch off coffee supply
	// ...

	setMachineState(machineState_off);
	// TODO: Notify user interface
}

static State offState = {
	.stateIndex = machineState_off,
	.entryAction = offStateEntryAction
};

// -----------------------------------------------------------------------------
// Initializing state
// -----------------------------------------------------------------------------

static void initializingStateEntryAction() {
	setMachineState(machineState_initializing);

	// Notifiy client
	sendNotification_BEGIN(this, MainController, clientDescriptor, MachineStateChangedNotification)
		.state = machineState_initializing
	sendNotification_END

	// Switch on coffee supply
	// ...
	// Switch on water supply
	sendRequest_BEGIN(this, WaterSupply, InitCommand)
	sendRequest_END
	// Switch on milk supply
	sendRequest_BEGIN(this, MilkSupply, InitCommand)
	sendRequest_END
}

static Event initializingStateDoAction() {
	return event_isInitialized;
}

static State initializingState = {
	.stateIndex = machineState_initializing,
	.entryAction = initializingStateEntryAction,
	.doAction = initializingStateDoAction
};

// -----------------------------------------------------------------------------
// Idle state
// -----------------------------------------------------------------------------

static void idleStateEntryAction() {
	logInfo("[mainController] Idle... awaiting command...");

	setMachineState(machineState_idle);

	// Notifiy client
	sendNotification_BEGIN(this, MainController, clientDescriptor, MachineStateChangedNotification)
		.state = machineState_idle
	sendNotification_END
}

static State idleState = {
	.stateIndex = machineState_idle,
	.entryAction = idleStateEntryAction
};

// -----------------------------------------------------------------------------
// Producing state
// -----------------------------------------------------------------------------

static int producingStatePrecondition() {
	// Only start production if...
	// - no coffee making process is already running (Paranoia)
	// - coffee is available
	// - water is available
	// - milk is not required or
	//     milk is available
	// - selected product is defined
	return !coffeeMaker.ongoingCoffeeMaking
		//TODO Activate these conditions
		//&& coffeeMaker.isCoffeeAvailable == available
		//&& coffeeMaker.isWaterAvailable == available
		//&& (!produceWithMilk || (coffeeMaker.isMilkAvailable == available))
		&& productToProduceIndex < getNumberOfProducts();
}

static void startMakeCoffeeProcess(unsigned int productIndex) {
	coffeeMaker.ongoingCoffeeMaking = newObject(&(MakeCoffeeProcessInstance) {
		.productIndex = productIndex,
		.withMilk = produceWithMilk
	}, sizeof(MakeCoffeeProcessInstance));

	setUpStateMachine(&coffeeMakingProcessMachine);
}

static void producingStateEntryAction() {
	setMachineState(machineState_producing);

	// Notifiy client
	sendNotification_BEGIN(this, MainController, clientDescriptor, MachineStateChangedNotification)
		.state = machineState_producing
	sendNotification_END

	startMakeCoffeeProcess(productToProduceIndex);
}

static void abortMakeCoffeeProcessInstance() {
	if (coffeeMaker.ongoingCoffeeMaking) {
		abortStateMachine(&coffeeMakingProcessMachine);

		deleteObject(coffeeMaker.ongoingCoffeeMaking);
		coffeeMaker.ongoingCoffeeMaking = NULL;
	}
}

static void producingStateExitAction() {
	abortMakeCoffeeProcessInstance();
}

static State producingState = {
	.stateIndex = machineState_producing,
	.precondition = producingStatePrecondition,
	.entryAction = producingStateEntryAction,
	.exitAction = producingStateExitAction
};

// -----------------------------------------------------------------------------
// State transitions
// -----------------------------------------------------------------------------

static StateMachine stateMachine = {
	.name = "mainController",
	.numberOfEvents = 7,
	.initialState = &offState,
	.transitions = {
		/* coffeeMaker_off: */
			/* event_switchedOn: */ &initializingState,
			/* event_switchedOff: */ NULL,
			/* event_isInitialized: */ NULL,
			/* event_productSelected: */ NULL,
			/* event_productionProcessAborted: */ NULL,
			/* event_productionProcessIsFinished: */ NULL,
			/* event_ingredientTankIsEmpty: */ NULL,
		/* coffeeMaker_initializing: */
			/* event_switchedOn: */ NULL,
			/* event_switchedOff: */ NULL,
			/* event_isInitialized: */ &idleState,
			/* event_productSelected: */ NULL,
			/* event_productionProcessAborted: */ NULL,
			/* event_productionProcessIsFinished: */ NULL,
			/* event_ingredientTankIsEmpty: */ NULL,
		/* coffeeMaker_idle: */
			/* event_switchedOn: */ NULL,
			/* event_switchedOff: */ &offState,
			/* event_isInitialized: */ NULL,
			/* event_productSelected: */ &producingState,
			/* event_productionProcessAborted: */ NULL,
			/* event_productionProcessIsFinished: */ NULL,
			/* event_ingredientTankIsEmpty: */ NULL,
		/* coffeeMaker_producing: */
			/* event_switchedOn: */ NULL,
			/* event_switchedOff: */ &offState,
			/* event_isInitialized: */ NULL,
			/* event_productSelected: */ NULL,
			/* event_productionProcessAborted: */ &idleState,
			/* event_productionProcessIsFinished: */ &idleState,
			/* event_ingredientTankIsEmpty: */ &idleState
		}
};

// =============================================================================
// 'Make coffee' process
// =============================================================================

// -----------------------------------------------------------------------------
// Events
// -----------------------------------------------------------------------------

/**
 * Represents a coffee making event.
 */
typedef enum {
	coffeeMakingEvent_isWarmedUp,
	coffeeMakingEvent_cupIsEmpty,
	coffeeMakingEvent_cupIsNotEmpty,
	coffeeMakingEvent_grindCoffeePowder,
	coffeeMakingEvent_coffeePowderGrinded,
	coffeeMakingEvent_supplyWater,
	coffeeMakingEvent_waterSupplied,
	coffeeMakingEvent_supplyMilk,
	coffeeMakingEvent_milkSupplied,
	coffeeMakingEvent_ejectCoffeeWaste,
	coffeeMakingEvent_coffeeWasteEjected,
	coffeeMakingEvent_errorOccured
} CoffeeMakingEvent;

// -----------------------------------------------------------------------------
// Warming Up activity
// -----------------------------------------------------------------------------

static void warmingUpActivityEntryAction() {
	logInfo("[mainController] [makeCoffee process] Warming up...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_warmingUp;
}

static Event warmingUpActivityDoAction() {
	return coffeeMakingEvent_isWarmedUp;
}

static State warmingUpActivity = {
	.stateIndex = coffeeMakingActivity_warmingUp,
	.entryAction = warmingUpActivityEntryAction,
	.doAction = warmingUpActivityDoAction
};

// -----------------------------------------------------------------------------
// Checking Cup Fill State activity
// -----------------------------------------------------------------------------

static Event checkingCupFillStateActivityDoAction() {
	logInfo("[mainController] [makeCoffee process] Checking cup fill state...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_checkingCupFillState;

	if (readNonBlockingDevice("./dev/cupFillStateSensor") > 0) {
		return coffeeMakingEvent_cupIsNotEmpty;
	}

	return coffeeMakingEvent_cupIsEmpty;
}

static State checkingCupFillStateActivity = {
	.stateIndex = coffeeMakingActivity_checkingCupFillState,
	.doAction = checkingCupFillStateActivityDoAction
};

// -----------------------------------------------------------------------------
// Grinding Coffee Powder activity
// -----------------------------------------------------------------------------

static void grindingCoffeePowderActivityEntryAction() {
	logInfo("[mainController] [makeCoffee process] Going to grind coffee powder...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_grindingCoffeePowder;

	//TODO Send grind coffee powder request to coffee supply
}

static void grindingCoffeePowderActivityExitAction() {

}

static State grindingCoffeePowderActivity = {
	.stateIndex = coffeeMakingActivity_grindingCoffeePowder,
	.entryAction = grindingCoffeePowderActivityEntryAction,
	.exitAction = grindingCoffeePowderActivityExitAction
};

// -----------------------------------------------------------------------------
// Supplying Water activity
// -----------------------------------------------------------------------------

static void supplyingWaterActivityEntryAction() {
	logInfo("[mainController] [makeCoffee process] Going to supply water...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_supplyingWater;

	sendRequest_BEGIN(this, WaterSupply, SupplyWaterCommand)
		//TODO Determine water amount on the basis of the product definition
		.waterAmount = coffeeMaker.ongoingCoffeeMaking->productIndex * 100
	sendRequest_END
}

static void supplyingWaterActivityExitAction() {

}

static State supplyingWaterActivity = {
	.stateIndex = coffeeMakingActivity_supplyingWater,
	.entryAction = supplyingWaterActivityEntryAction,
	.exitAction = supplyingWaterActivityExitAction
};

// -----------------------------------------------------------------------------
// With Milk gateway
// -----------------------------------------------------------------------------

static Event withMilkGatewayDoAction() {
	if (coffeeMaker.ongoingCoffeeMaking->withMilk) {
		return coffeeMakingEvent_supplyMilk;
	} else {
		return coffeeMakingEvent_ejectCoffeeWaste;
	}

	return NO_EVENT;
}

static State withMilkGateway = {
	.stateIndex = coffeeMakingActivity_withMilkGateway,
	.doAction = withMilkGatewayDoAction
};

// -----------------------------------------------------------------------------
// Supplying Milk activity
// -----------------------------------------------------------------------------

static void supplyingMilkActivityEntryAction() {
	logInfo("[mainController] [makeCoffee process] Going to supply milk...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_supplyingMilk;

	sendRequest_BEGIN(this, MilkSupply, SupplyMilkCommand)
		//TODO Determine milk amount on the basis of the product definition
		.milkAmount = 20
	sendRequest_END
}

static void supplyingMilkActivityExitAction() {

}

static State supplyingMilkActivity = {
	.stateIndex = coffeeMakingActivity_supplyingMilk,
	.entryAction = supplyingMilkActivityEntryAction,
	.exitAction = supplyingMilkActivityExitAction
};

// -----------------------------------------------------------------------------
// Ejecting Coffee Waste activity
// -----------------------------------------------------------------------------

static void ejectingCoffeeWasteActivityEntryAction() {
	logInfo("[mainController] [makeCoffee process] Going to eject coffee waste...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_ejectingCoffeeWaste;

	//TODO Send eject coffee waste request to coffee supply
}

static void ejectingCoffeeWasteActivityExitAction() {

}

static State ejectingCoffeeWasteActivity = {
	.stateIndex = coffeeMakingActivity_ejectingCoffeeWaste,
	.entryAction = ejectingCoffeeWasteActivityEntryAction,
	.exitAction = ejectingCoffeeWasteActivityExitAction
};

// -----------------------------------------------------------------------------
// Finished state
// -----------------------------------------------------------------------------

static void finishedStateEntryAction() {
	logInfo("[mainController] [makeCoffee process] Finished.");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_finished;

	processStateMachineEvent(&stateMachine, event_productionProcessIsFinished);
}

static State finishedState = {
	.stateIndex = coffeeMakingActivity_finished,
	.entryAction = finishedStateEntryAction
};

// -----------------------------------------------------------------------------
// Error state
// -----------------------------------------------------------------------------

static void errorStateEntryAction() {
	logErr("[mainController] [makeCoffee process] Error occured! Aborting...");

	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_error;

	processStateMachineEvent(&stateMachine, event_productionProcessAborted);
}

static State errorState = {
	.stateIndex = coffeeMakingActivity_error,
	.entryAction = errorStateEntryAction
};

// -----------------------------------------------------------------------------
// Activity/state transitions
// -----------------------------------------------------------------------------
static StateMachine coffeeMakingProcessMachine = {
	.name = "coffeeMakingProcess",
	.numberOfEvents = 12,
	.initialState = &warmingUpActivity,
	.transitions = {
		/* coffeeMakingActivity_warmingUp: */
			/* coffeeMakingEvent_isWarmedUp: */ &checkingCupFillStateActivity,
			/* coffeeMakingEvent_cupIsEmpty: */ NULL,
			/* coffeeMakingEvent_cubIsNotEmpty: */ NULL,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ NULL,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ NULL,
			/* coffeeMakingEvent_supplyMilk: */ NULL,
			/* coffeeMakingEvent_milkSupplied: */ NULL,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ NULL,
			/* coffeeMakingEvent_coffeeWasteEjected: */ NULL,
			/* coffeeMakingEvent_errorOccured: */ NULL,
		/* coffeeMakingActivity_checkingCupFillState: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_cupIsEmpty: */ &supplyingWaterActivity, //&grindingCoffeePowderActivity,
			/* coffeeMakingEvent_cubIsNotEmpty: */ &errorState,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ NULL,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ NULL,
			/* coffeeMakingEvent_supplyMilk: */ NULL,
			/* coffeeMakingEvent_milkSupplied: */ NULL,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ NULL,
			/* coffeeMakingEvent_coffeeWasteEjected: */ NULL,
			/* coffeeMakingEvent_errorOccured: */ NULL,
		/* coffeeMakingActivity_grindingCoffeePowder: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_cupIsEmpty: */ NULL,
			/* coffeeMakingEvent_cubIsNotEmpty: */ NULL,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ &supplyingWaterActivity,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ NULL,
			/* coffeeMakingEvent_supplyMilk: */ NULL,
			/* coffeeMakingEvent_milkSupplied: */ NULL,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ NULL,
			/* coffeeMakingEvent_coffeeWasteEjected: */ NULL,
			/* coffeeMakingEvent_errorOccured: */ &errorState,
		/* coffeeMakingActivity_supplyingWater: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_cupIsEmpty: */ NULL,
			/* coffeeMakingEvent_cubIsNotEmpty: */ NULL,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ NULL,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ &withMilkGateway,
			/* coffeeMakingEvent_supplyMilk: */ NULL,
			/* coffeeMakingEvent_milkSupplied: */ NULL,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ NULL,
			/* coffeeMakingEvent_coffeeWasteEjected: */ NULL,
			/* coffeeMakingEvent_errorOccured: */ &errorState,
		/* coffeeMakingActivity_withMilkGateway: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_cupIsEmpty: */ NULL,
			/* coffeeMakingEvent_cubIsNotEmpty: */ NULL,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ NULL,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ NULL,
			/* coffeeMakingEvent_supplyMilk: */ &supplyingMilkActivity,
			/* coffeeMakingEvent_milkSupplied: */ NULL,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ &ejectingCoffeeWasteActivity,
			/* coffeeMakingEvent_coffeeWasteEjected: */ NULL,
			/* coffeeMakingEvent_errorOccured: */ NULL,
		/* coffeeMakingActivity_supplyingMilk: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_cupIsEmpty: */ NULL,
			/* coffeeMakingEvent_cubIsNotEmpty: */ NULL,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ NULL,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ NULL,
			/* coffeeMakingEvent_supplyMilk: */ NULL,
			/* coffeeMakingEvent_milkSupplied: */ &finishedState, //&ejectingCoffeeWasteActivity,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ NULL,
			/* coffeeMakingEvent_coffeeWasteEjected: */ NULL,
			/* coffeeMakingEvent_errorOccured: */ &errorState,
		/* coffeeMakingActivity_ejectingCoffeeWaste: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_cupIsEmpty: */ NULL,
			/* coffeeMakingEvent_cubIsNotEmpty: */ NULL,
			/* coffeeMakingEvent_grindCoffeePowder: */ NULL,
			/* coffeeMakingEvent_coffeePowderGrinded: */ NULL,
			/* coffeeMakingEvent_supplyWater: */ NULL,
			/* coffeeMakingEvent_waterSupplied: */ NULL,
			/* coffeeMakingEvent_supplyMilk: */ NULL,
			/* coffeeMakingEvent_milkSupplied: */ NULL,
			/* coffeeMakingEvent_ejectCoffeeWaste: */ NULL,
			/* coffeeMakingEvent_coffeeWasteEjected: */ &finishedState,
			/* coffeeMakingEvent_errorOccured: */ &errorState,
		}
};

static void setUpMainController(void *activity) {
	logInfo("[mainController] Setting up...");

	this = (Activity *)activity;
}

static void runMainController(void *activity) {
	logInfo("[mainController] Running...");

	// Water supply test
	/*
	{
		sleep(1);

		logInfo("[mainController] Going to switch on water supply...");
		sendRequest_BEGIN(this, WaterSupply, InitCommand)
		sendRequest_END

		sleep(1);

		logInfo("[mainController] Going to supply water...");
		sendRequest_BEGIN(this, WaterSupply, SupplyWaterCommand)
			.waterAmount = 200
		sendRequest_END

		while (TRUE) {
			receiveGenericMessage_BEGIN(this)
				if (result > 0) {
					MESSAGE_SELECTOR_BEGIN
						MESSAGE_BY_SENDER_SELECTOR(senderDescriptor, message, WaterSupply)
							MESSAGE_SELECTOR_BEGIN
								MESSAGE_BY_TYPE_SELECTOR(*specificMessage, WaterSupply, Result)
									if (content.code == OK_RESULT) {
										logInfo("[mainController] OK Result from water supply received!");
									} else {
										logInfo("[mainController] Unexpected result from water supply received: %u", content.code);
									}
								MESSAGE_BY_TYPE_SELECTOR(*specificMessage, WaterSupply, Status)
									if (content.availability == available) {
										logInfo("[mainController] 'Water available' status from water supply received!");
									} else {
										logInfo("[mainController] 'Water not available' status from water supply received!");
									}
								MESSAGE_SELECTOR_ANY
									logWarn("[mainController] Irrelevant message with content id %u from water supply received!", specificMessage->type);
							MESSAGE_SELECTOR_END
						MESSAGE_SELECTOR_ANY
							logWarn("[mainController] Irrelevant message from %s received!", senderDescriptor.name);
					MESSAGE_SELECTOR_END
				}
			receiveGenericMessage_END
		}
	}
	*/

	// Coffee supply test
	/*
	sleep(1);

	logInfo("[mainController] Going to switch on coffee supply...");
	sendMessage(getCoffeeSupplyDescriptor(), (char *)&(CoffeeSupplyMessage) {
		.activity = getMainControllerDescriptor(),
		.intValue = INIT_COMMAND
	}, sizeof(CoffeeSupplyMessage), messagePriority_medium);

	sleep(1);

	logInfo("[mainController] Going to grind beans...");
	sendMessage(getCoffeeSupplyDescriptor(), (char *)&(CoffeeSupplyMessage) {
		.activity = getMainControllerDescriptor(),
		.intValue = SUPPLY_START_COMMAND
	}, sizeof(CoffeeSupplyMessage), messagePriority_medium);

	while (TRUE);
	*/

	// Milk supply test
	/*
	sleep(1);

	sendRequest_BEGIN(this, MilkSupply, SupplyMilkCommand)
		.milkAmount = 20
	sendRequest_END

	while (TRUE);
	*/

	setUpStateMachine(&stateMachine);

	// Main controller test
	///*
	sleep(1);

	logInfo("-------------------------------------------------------");

	// Switch on main controller
	sendRequest_BEGIN(this, MainController, InitCommand)
	sendRequest_END

	sleep(1);

	// Product one (or two) products
	sendRequest_BEGIN(this, MainController, ProduceProductCommand)
		.productIndex = 1,
		.withMilk = TRUE
	sendRequest_END
//	sendRequest_BEGIN(this, MainController, ProduceProductCommand)
//		.productIndex = 2,
//		.withMilk = TRUE
//	sendRequest_END

	sleep(1);

	// Switch off main controller
	//sendRequest_BEGIN(this, MainController, OffCommand)
	//sendRequest_END
	//*/

	while (TRUE) {
		receiveGenericMessage_BEGIN(this)
			if (result > 0) {
				MESSAGE_SELECTOR_BEGIN
					MESSAGE_BY_SENDER_SELECTOR(senderDescriptor, message, WaterSupply)
						MESSAGE_SELECTOR_BEGIN
							// If we got a result from water supply...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, WaterSupply, Result)
								// Propagate event to coffee making process state machine
								if (content.code == OK_RESULT) {
									processStateMachineEvent(&coffeeMakingProcessMachine, coffeeMakingEvent_waterSupplied);
								} else {
									processStateMachineEvent(&coffeeMakingProcessMachine, coffeeMakingEvent_errorOccured);
								}
							// If we got a status update from water supply...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, WaterSupply, Status)
								coffeeMaker.isWaterAvailable = content.availability;

								// Send notification to client
								sendNotification_BEGIN(this, MainController, clientDescriptor, IngredientAvailabilityChangedNotification)
									.ingredientIndex = WATER_INDEX,
									.availability = coffeeMaker.isWaterAvailable
								sendNotification_END
							MESSAGE_SELECTOR_ANY

						MESSAGE_SELECTOR_END
					MESSAGE_BY_SENDER_SELECTOR(senderDescriptor, message, MilkSupply)
						MESSAGE_SELECTOR_BEGIN
							// If we got a result from milk supply...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, MilkSupply, Result)
								// Propagate event to coffee making process state machine
								if (content.code == OK_RESULT) {
									processStateMachineEvent(&coffeeMakingProcessMachine, coffeeMakingEvent_milkSupplied);
								} else {
									processStateMachineEvent(&coffeeMakingProcessMachine, coffeeMakingEvent_errorOccured);
								}
							// If we got a status update from milk supply...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, MilkSupply, Status)
								coffeeMaker.isMilkAvailable = content.availability;

								// Send notification to client
								sendNotification_BEGIN(this, MainController, clientDescriptor, IngredientAvailabilityChangedNotification)
									.ingredientIndex = MILK_INDEX,
									.availability = coffeeMaker.isMilkAvailable
								sendNotification_END
							MESSAGE_SELECTOR_ANY

						MESSAGE_SELECTOR_END
					MESSAGE_SELECTOR_ANY
						MainControllerMessage *specificMessage = (MainControllerMessage *)&message;
						MESSAGE_SELECTOR_BEGIN
							// If we got an init command...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, MainController, InitCommand)
								logInfo("[mainController] Going to switch on...");

								clientDescriptor = senderDescriptor;

								processStateMachineEvent(&stateMachine, event_switchedOn);
							// If we got an off command...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, MainController, OffCommand)
								logInfo("[mainController] Going to switch off...");

								processStateMachineEvent(&stateMachine, event_switchedOff);
							// If we got an produce product command...
							MESSAGE_BY_TYPE_SELECTOR(*specificMessage, MainController, ProduceProductCommand)
								logInfo("[mainController] Going to produce product %u %s milk...", content.productIndex, (content.withMilk ? "with" : "without"));

								productToProduceIndex = content.productIndex;
								produceWithMilk = content.withMilk;

								processStateMachineEvent(&stateMachine, event_productSelected);
							MESSAGE_SELECTOR_ANY
								logWarn("[mainController] Unexpected message from %s received!", senderDescriptor.name);
						MESSAGE_SELECTOR_END
				MESSAGE_SELECTOR_END
			}
		receiveGenericMessage_END
	}
}

static void tearDownMainController(void *activity) {
	logInfo("[mainController] Tearing down...");
}
