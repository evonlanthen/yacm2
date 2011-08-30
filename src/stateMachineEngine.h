/**
 * State machine engine
 *
 * Contains the state machine engine.
 *
 * @file    stateMachineEngine.h
 * @version 0.1
 * @author  Ronny Stauffer (staur3@bfh.ch)
 * @date    Jun 4, 2011
 */

#ifndef STATEMACHINEENGINE_H_
#define STATEMACHINEENGINE_H_

/**
 * Special case value for 'no event'.
 */
#define NO_EVENT 999

/**
 * Represents an event.
 */
typedef int Event;

/**
 * Defines the signature of a state precondition predicate.
 */
typedef int (*StatePrecondition)();
/**
 * Defines the signature of a state action.
 */
typedef void (*StateAction)();
/**
 * Defines the signature of a 'do' state action.
 */
typedef Event (*DoStateAction)();

/**
 * Represents a state.
 */
typedef struct {
	int stateIndex; /**< The state's index. */
	StatePrecondition precondition; /**< The state's precondition predicate determines if a state can be activated or not. If all preconditions to activate the state are met the predicate should return TRUE, otherwise FALSE. */
	StateAction entryAction; /**< The state's 'entry' action is called once after the state was activated. */
	DoStateAction doAction; /**< The state's 'do' action is constantly called while the state is active. */
	StateAction exitAction; /**< The state's 'exit' action is called once before the state will be deactivated. */
} State;

/**
 * Represents a state machine definition.
 */
typedef struct {
	int isInitialized; /**< Is the state machine already initialized? */
	unsigned int numberOfEvents; /**<  The number of defined events. */
	State *initialState; /**< Defines the state machine's initial state. */
	State *activeState; /**< The current state. */
	State *transitions[]; /**< Defines the state machine's state transitions. */
} StateMachine;

/**
 * Sets up and starts a new state machine.
 *
 * @param stateMachine A state machine definition.
 */
extern void setUpStateMachine(StateMachine *stateMachine);

/**
 * Heartbeat function for ongoing tasks.
 * Should be constantly called by the client.
 *
 * @param stateMachine A state machine definition.
 */
extern void runStateMachine(StateMachine *stateMachine);

/**
 * Aborts a running state machine.
 *
 * @param stateMachine A state machine definition.
 */
extern void abortStateMachine(StateMachine *stateMachine);

/**
 * Signals an event to a state machine.
 *
 * @param stateMachine A state machine definition.
 * @param event An event.
 */
extern void processStateMachineEvent(StateMachine *stateMachine, Event event);

#endif /* STATEMACHINEENGINE_H_ */
