/**
 * @brief   User interface
 * @file    userInterface.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

/*
 * Benutzerfuehrung: Controller (Main Thread)
 * Ausgabe: Output
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "display.h"
#include "mainController.h"
#include "userInterface.h"

static void setUpUserInterface(void *activity);
static void runUserInterface(void *activity);
static void tearDownUserInterface(void *activity);

static Activity *display;

static ActivityDescriptor userInterfaceDescriptor = {
	.name = "userInterface",
	.setUp = setUpUserInterface,
	.run = runUserInterface,
	.tearDown = tearDownUserInterface
};

ActivityDescriptor getUserInterfaceDescriptor() {
	return userInterfaceDescriptor;
}

static void setUpUserInterface(void *activity) {
	logInfo("[userInterface] Setting up...");
	display = createActivity(getDisplayDescriptor(), messageQueue_blocking);
}

static void runUserInterface(void *activity) {
	UserInterfaceMessage incomingMessage;
	//int incomingMessageLength;
	MainControllerMessage mainControllerMessage;
	mainControllerMessage.activity = getUserInterfaceDescriptor();

	logInfo("[userInterface] Running...");

	while (TRUE) {
		int result = waitForEvent(activity, (char *)&incomingMessage, sizeof(incomingMessage), 3000);
		if (result < 0) {
			sleep(10);
			continue;
		}

		// Check if there is an incoming message
		if (result > 0) {
			// Process incoming message
			MESSAGE_SELECTOR_BEGIN
			MESSAGE_SELECTOR(incomingMessage, mainController)
				logInfo("[userInterface] Send message to display...");
				sendMessage(getDisplayDescriptor(), (char *)&(DisplayMessage) {
					.activity = getUserInterfaceDescriptor(),
					.intValue = 2,
					.strValue = "Show view 2",
				}, sizeof(DisplayMessage), messagePriority_medium);
			MESSAGE_SELECTOR(incomingMessage, display)
				logInfo("[userInterface] Send message to mainController (%d, %s)...", incomingMessage.intValue, incomingMessage.strValue);
				mainControllerMessage.intValue = incomingMessage.intValue;
				strcpy(mainControllerMessage.strValue, incomingMessage.strValue);
				sendMessage(getMainControllerDescriptor(),
					(char *)&mainControllerMessage,
					sizeof(mainControllerMessage),
					messagePriority_medium);
			MESSAGE_SELECTOR_END
		}

		// Perform heartbeat tasks
		//...
	}
}

static void tearDownUserInterface(void *activity) {
	logInfo("[userInterface] Tearing down...");
	destroyActivity(display);
}

/*
int waitForEvent(Activity *activity, char *buffer, unsigned long length, unsigned int timeout) {
	//logInfo("[%s] Going to wait for an event...", activity->descriptor->name);

	int polling;
	if ((polling = epoll_create(1)) < 0) {
		logErr("[%s] Error setting up event waiting: %s", activity->descriptor->name, strerror(errno));

		return -EFAULT;
	}

	struct epoll_event messageQueueEventDescriptor = {
			.events = EPOLLIN,
			.data.fd = activity->messageQueue
	};
	if (epoll_ctl(polling, EPOLL_CTL_ADD, messageQueueEventDescriptor.data.fd, &messageQueueEventDescriptor) < 0) {
		logErr("[%s] Error registering message queue event source: %s", activity->descriptor->name, strerror(errno));

		close(polling);

		return -EFAULT;
	}

	struct epoll_event firedEvents[1];
	int numberOfFiredEvents;
	numberOfFiredEvents = epoll_wait(polling, firedEvents, 1, timeout);

	close(polling);

	if (numberOfFiredEvents < 0) {
		logErr("[%s] Error waiting for event: %s", activity->descriptor->name, strerror(errno));

		return -EFAULT;
	} else if (numberOfFiredEvents == 1) {
		//logInfo("[%s] Message received!", activity->descriptor->name);

		unsigned long incomingMessageLength = receiveMessage(activity, buffer, length);

		return incomingMessageLength;
	} else {
		//logInfo("[%s] Timeout occured!", activity->descriptor->name);

		return 0;
	}
}
*/
