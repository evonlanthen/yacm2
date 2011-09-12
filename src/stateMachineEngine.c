/**
 * @file   stateMachineEngine.c
 * @author Ronny Stauffer (staur3@bfh.ch)
 * @date   Jun 4, 2011
 * @brief  Contains the state machine engine.
 *
 * Contains the state machine engine.
 */

#include "defines.h"
#include "syslog.h"
#include "stateMachineEngine.h"

static Event activateState(StateMachine *stateMachine, State *nextState);
static Event runState(State *state);

/**
 * @copydoc setUpStateMachine
 */
void setUpStateMachine(StateMachine *stateMachine) {
	if (stateMachine->isInitialized) {
		return;
	}

	activateState(stateMachine, stateMachine->initialState);

	stateMachine->isInitialized = TRUE;
}

/**
 * @copydoc setUpStateMachine
 */
void runStateMachine(StateMachine *stateMachine) {
	if (!stateMachine->isInitialized) {
		return;
	}

	// Run active state and process events
	Event event = runState(stateMachine->activeState);
	if (event != NO_EVENT) {
		processStateMachineEvent(stateMachine, event);
	}
}

/**
 * @copydoc abortStateMachine
 */
void abortStateMachine(StateMachine *stateMachine) {
	if (!stateMachine->isInitialized) {
		return;
	}

	if (stateMachine->activeState->exitAction) {
		stateMachine->activeState->exitAction();
	}

	stateMachine->isInitialized = FALSE;
}

/**
 * @copydoc processStateMachineEvent
 */
void processStateMachineEvent(StateMachine *stateMachine, Event event) {
	if (!stateMachine->isInitialized) {
		return;
	}

	// Processing an event means looking up the state machine's next state in the transition table
	State *nextState = stateMachine->transitions[stateMachine->activeState->stateIndex * stateMachine->numberOfEvents + event];
	if (nextState) {
		// If next state either has no precondition
		// or the precondition is true...
		if (!nextState->precondition
			|| nextState->precondition()) {
			// Activate next state
			Event event = activateState(stateMachine, nextState);
			if (event != NO_EVENT) {
				processStateMachineEvent(stateMachine, event);
			}
		} else {
			logWarn("[%s state machine] Precondition of state %d is not met!", stateMachine->name, nextState->stateIndex);
		}
	} else {
		logWarn("[%s state machine] Ignoring event %d!", stateMachine->name, event);
	}
}

/**
 * Heartbeat function for the state's 'do' action.
 */
static Event runState(State *state) {
	Event event = NO_EVENT;

	// If the state has a 'do' action, then run it
	if (state->doAction) {
		event = state->doAction();
	}

	return event;
}

/**
 * Activates the given state.
 */
static Event activateState(StateMachine *stateMachine, State *nextState) {
	// If a state is currently active and the state has an exit action,
	// then run the state's exit action
	if (stateMachine->activeState) {
		if (stateMachine->activeState->exitAction) {
			stateMachine->activeState->exitAction();
		}
	}
	// Make the next state the currently active state
	stateMachine->activeState = nextState;
	// If the (now currently active) state has an entry action,
	// run the state's entry action
	if (stateMachine->activeState->entryAction) {
		stateMachine->activeState->entryAction();
	}

	// If the state has a 'do' action, then run it
	Event event = NO_EVENT;
	if (stateMachine->activeState->doAction) {
		event = stateMachine->activeState->doAction();
	}

	return event;
}
