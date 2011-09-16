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
MESSAGE_CONTENT_TYPE_MAPPING(MainController, ProduceProductCommand, 2)

static Activity *this;

ActivityDescriptor getMainControllerDescriptor() {
	return mainControllerDescriptor;
}

/**
 * Represents the milk preselection state.
 */
typedef enum {
  milkPreselection_off,
  milkPreselection_on
} MilkPreselectionState;

/**
 * Represents an activity (= state) within the coffee making process.
 */
typedef enum {
	coffeeMakingActivity_warmingUp,
	coffeeMakingActivity_withMilkGateway,
	coffeeMakingActivity_deliveringMilk,
	coffeeMakingActivity_deliveringCoffee,
	coffeeMakingActivity_finished,
	coffeeMakingActivity_error,
	coffeeMakingActivity_undefined
} CoffeeMakingActivity;

/**
 * Represents an ongoing coffee making process instance.
 */
typedef struct {
	//Product *product; /**< The product currently produced. */
	int withMilk; /**< Is the product produced with milk? */
	CoffeeMakingActivity currentActivity; /**< The activity which is currently executed. */
} MakeCoffeeProcessInstance;

/**
 * Represents the coffee maker.
 */
typedef struct {
	MachineState state; /**< The coffee maker's state. */
	//Coffee coffee; /**< The coffee ingredient. */
	//Milk milk; /**< The milk ingredient. */
	//ProductListElement *products; /**< The product definition collection. */
	MilkPreselectionState milkPreselectionState; /**< The milk preselection state. */
	MakeCoffeeProcessInstance *ongoingCoffeeMaking; /**< A possibly ongoing coffee making process instance. */
} CoffeeMaker;

/**
 * The coffee maker model instance.
 */
static CoffeeMaker coffeeMaker = {
		.state = machineState_off,
		//.coffee.isAvailable = TRUE,
		//.coffee.emptyTankSensorId = SENSOR_1,
		//.milk.isAvailable = TRUE,
		//.milk.emptyTankSensorId = SENSOR_2,
		.milkPreselectionState = milkPreselection_off
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
	state_switchedOn,
	state_switchedOff,
	state_isInitialized,
	state_productSelected,
	state_productionProcessAborted,
	state_productionProcessIsFinished,
	state_ingredientTankIsEmpty
} MainControllerState;

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
	// TODO: Notify user interface
}

static State initializingState = {
	.stateIndex = machineState_initializing,
	.entryAction = initializingStateEntryAction,
};

// -----------------------------------------------------------------------------
// Idle state
// -----------------------------------------------------------------------------

static void idleStateEntryAction() {
	setMachineState(machineState_idle);
	// TODO: Notify user interface
}

static State idleState = {
	.stateIndex = machineState_idle,
	.entryAction = idleStateEntryAction
};

// -----------------------------------------------------------------------------
// Producing state
// -----------------------------------------------------------------------------

/*
static int producingStatePrecondition()coffeeMakingProcessMachine {
	// Only start production if...
	// - no coffee making process is already running
	// - coffee is available (coffee tank is not empty)
	// - milk preselection is off or
	//   milk is available (milk tank is not emtpy)
	// - selected product is defined
	return !coffeeMaker.ongoingCoffeeMaking
		&& coffeeMaker.coffee.isAvailable
		&& (coffeeMaker.milkPreselectionState != milkPreselection_on || coffeeMaker.milk.isAvailable)
		&& selectedProductIndex < getNumberOfProducts();
}*/

static void startMakeCoffeeProcess(unsigned int productIndex) {
	coffeeMaker.ongoingCoffeeMaking = newObject(&(MakeCoffeeProcessInstance) {
		//.product = getProduct(productIndex),
		.withMilk = coffeeMaker.milkPreselectionState == milkPreselection_on ? TRUE : FALSE
	}, sizeof(MakeCoffeeProcessInstance));

	setUpStateMachine(&coffeeMakingProcessMachine);
}

static void producingStateEntryAction() {
	//startMakeCoffeeProcess(selectedProductIndex);
	setMachineState(machineState_producing);
	// TODO: Notify user interface
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
	// TODO: Notify user interface
}

static State producingState = {
	.stateIndex = machineState_producing,
	//.precondition = producingStatePrecondition,
	.entryAction = producingStateEntryAction,
	.exitAction = producingStateExitAction
};

// -----------------------------------------------------------------------------
// State transitions
// -----------------------------------------------------------------------------

static StateMachine stateMachine = {
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
	coffeeMakingEvent_deliverMilk,
	coffeeMakingEvent_milkDelivered,
	coffeeMakingEvent_deliverCoffee,
	coffeeMakingEvent_coffeeDelivered,
	coffeeMakingEvent_ingredientTankIsEmpty
} CoffeeMakingEvent;

// -----------------------------------------------------------------------------
// Warming Up activity
// -----------------------------------------------------------------------------

static void warmingUpActivityEntryAction() {
	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_warmingUp;
	// TODO: Notify user interface
}

static State warmingUpActivity = {
	.stateIndex = coffeeMakingActivity_warmingUp,
	.entryAction = warmingUpActivityEntryAction,
};

// -----------------------------------------------------------------------------
// With Milk gateway
// -----------------------------------------------------------------------------

static Event withMilkGatewayDoAction() {
	if (coffeeMaker.ongoingCoffeeMaking->withMilk) {
		return coffeeMakingEvent_deliverMilk;
	} else {
		return coffeeMakingEvent_deliverCoffee;
	}

	return NO_EVENT;
}

static State withMilkGateway = {
	.stateIndex = coffeeMakingActivity_withMilkGateway,
	.doAction = withMilkGatewayDoAction
};

// -----------------------------------------------------------------------------
// Delivering Milk activity
// -----------------------------------------------------------------------------

static void deliveringMilkActivityEntryAction() {
	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_deliveringMilk;
	// TODO: Notify milk supply and user interface
}

static void deliveringMilkActivityExitAction() {
	// TODO: Notify milk supply and user interface
}

static State deliveringMilkActivity = {
	.stateIndex = coffeeMakingActivity_deliveringMilk,
	.entryAction = deliveringMilkActivityEntryAction,
	.exitAction = deliveringMilkActivityExitAction
};

// -----------------------------------------------------------------------------
// Delivering Coffe activity
// -----------------------------------------------------------------------------

static void deliveringCoffeeActivityEntryAction() {
	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_deliveringCoffee;
	// TODO: Notify coffee supply and user interface
}

static void deliveringCoffeeActivityExitAction() {
	// TODO: Notify coffee supply and user interface
}

static State deliveringCoffeeActivity = {
	.stateIndex = coffeeMakingActivity_deliveringCoffee,
	.entryAction = deliveringCoffeeActivityEntryAction,
	.exitAction = deliveringCoffeeActivityExitAction
};

// -----------------------------------------------------------------------------
// Finished state
// -----------------------------------------------------------------------------

static void finishedStateEntryAction() {
	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_finished;
	// TODO: Notify user interface
}

static State finishedState = {
	.stateIndex = coffeeMakingActivity_finished,
	.entryAction = finishedStateEntryAction
};

// -----------------------------------------------------------------------------
// Error state
// -----------------------------------------------------------------------------

static void errorStateEntryAction() {
	coffeeMaker.ongoingCoffeeMaking->currentActivity = coffeeMakingActivity_error;
	// TODO: Notify user interface
}

static State errorState = {
	.stateIndex = coffeeMakingActivity_error,
	.entryAction = errorStateEntryAction
};

// -----------------------------------------------------------------------------
// Activity/state transitions
// -----------------------------------------------------------------------------
static StateMachine coffeeMakingProcessMachine = {
	.numberOfEvents = 6,
	.initialState = &warmingUpActivity,
	.transitions = {
		/* coffeeMakingActivity_warmingUp: */
			/* coffeeMakingEvent_isWarmedUp: */ &withMilkGateway,
			/* coffeeMakingEvent_deliverMilk: */ NULL,
			/* coffeeMakingEvent_milkDelivered: */ NULL,
			/* coffeeMakingEvent_deliverCoffee: */ NULL,
			/* coffeeMakingEvent_coffeeDelivered: */ NULL,
			/* coffeeMakingEvent_ingredientTankIsEmpty: */ NULL,
		/* coffeeMakingActivity_withMilkGateway: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_deliverMilk: */ &deliveringMilkActivity,
			/* coffeeMakingEvent_milkDelivered: */ NULL,
			/* coffeeMakingEvent_deliverCoffee: */ &deliveringCoffeeActivity,
			/* coffeeMakingEvent_coffeeDelivered: */ NULL,
			/* coffeeMakingEvent_ingredientTankIsEmpty: */ NULL,
		/* coffeeMakingActivity_deliveringMilk: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_deliverMilk: */ NULL,
			/* coffeeMakingEvent_milkDelivered: */ &deliveringCoffeeActivity,
			/* coffeeMakingEvent_deliverCoffee: */ NULL,
			/* coffeeMakingEvent_coffeeDelivered: */ NULL,
			/* coffeeMakingEvent_ingredientTankIsEmpty: */ &errorState,
		/* coffeeMakingActivity_deliveringCoffee: */
			/* coffeeMakingEvent_isWarmedUp: */ NULL,
			/* coffeeMakingEvent_deliverMilk: */ NULL,
			/* coffeeMakingEvent_milkDelivered: */ NULL,
			/* coffeeMakingEvent_deliverCoffee: */ NULL,
			/* coffeeMakingEvent_coffeeDelivered: */ &finishedState,
			/* coffeeMakingEvent_ingredientTankIsEmpty: */ &errorState
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
			ActivityDescriptor senderDescriptor;
			Byte message[400];
			int result = receiveMessage2(this, &senderDescriptor, &message, 400);
			if (result > 0) {
				MESSAGE_SELECTOR_BEGIN
					MESSAGE_BY_SENDER_SELECTOR(WaterSupply)
						WaterSupplyMessage *waterSupplyMessage = (WaterSupplyMessage *)&message;
						MESSAGE_SELECTOR_BEGIN
							MESSAGE_BY_TYPE_SELECTOR((*waterSupplyMessage), WaterSupply, Result)
								logInfo("[mainController] Result from water supply received: %u", content.code);
							MESSAGE_SELECTOR_ANY
								logWarn("[mainController] Unexpected message %u from water supply received!", waterSupplyMessage->type);
						MESSAGE_SELECTOR_END
					MESSAGE_SELECTOR_ANY
						logWarn("[mainController] Unexpected message from %s received!", senderDescriptor.name);
				MESSAGE_SELECTOR_END
			}
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

	sleep(10);
}

static void tearDownMainController(void *activity) {
	logInfo("[mainController] Tearing down...");
}
