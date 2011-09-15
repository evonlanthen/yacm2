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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"
#include "display.h"
#include "activity.h"
#include "mainController.h"
#include "userInterface.h"

static void setUpUserInterface(void *activity);
static void runUserInterface(void *activity);
static void tearDownUserInterface(void *activity);

static ActivityDescriptor userInterfaceDescriptor = {
	.name = "userInterface",
	.setUp = setUpUserInterface,
	.run = runUserInterface,
	.tearDown = tearDownUserInterface
};

MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, Command, 1)
MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, ChangeViewCommand, 2)
MESSAGE_CONTENT_TYPE_MAPPING(UserInterface, Status, 3)

static Activity *this;
static Activity *display;

ActivityDescriptor getUserInterfaceDescriptor() {
	return userInterfaceDescriptor;
}

static void setUpUserInterface(void *activity) {
	logInfo("[userInterface] Setting up...");
	display = createActivity(getDisplayDescriptor(), messageQueue_blocking);
	this = (Activity *)activity;
}

static void runUserInterface(void *activity) {
	int polling, value, fd, i, result;
	struct epoll_event firedEvents[100];
	int numberOfFiredEvents;
	char buffer[3];
	char buttonsDevice[] = "./dev/buttons";
	int buttonsFileDescriptor;
	char switchesDevice[] = "./dev/switches";
	int switchesFileDescriptor;
	MainControllerMessage mainControllerMessage;
	mainControllerMessage.activity = getUserInterfaceDescriptor();

	logInfo("[userInterface] Running...");

	// open file descriptors for buttons and switches:
	buttonsFileDescriptor = open(buttonsDevice, O_RDONLY);
	if (buttonsFileDescriptor < 0) {
		logErr("[%s] Unable to open device %s: %s", (*this->descriptor).name, buttonsDevice, strerror(errno));
	}
	switchesFileDescriptor = open(switchesDevice, O_RDONLY);
	if (switchesFileDescriptor < 0) {
		logErr("[%s] Unable to open device %s: %s", (*this->descriptor).name, switchesDevice, strerror(errno));
	}

	// create poll device:
	if ((polling = epoll_create(3)) < 0) {
		logErr("[%s] Error setting up event waiting: %s", (*this->descriptor).name, strerror(errno));
	}
	struct epoll_event eventDescriptors[] = {
		{ .events = EPOLLIN, .data.fd = 0 }, //(*this->messageQueue) },
		{ .events = EPOLLIN, .data.fd = buttonsFileDescriptor },
		{ .events = EPOLLIN, .data.fd = switchesFileDescriptor }
	};
	for (i = 0; i < 3; i++) {
		if (epoll_ctl(polling, EPOLL_CTL_ADD, eventDescriptors[i].data.fd, &eventDescriptors[i]) < 0) {
			logErr("[%s] Error registering event source %d: %s", (*this->descriptor).name, i, strerror(errno));
			close(polling);
			close(buttonsFileDescriptor);
			close(switchesFileDescriptor);
			return;
		}
	}

	while (TRUE) {
		// wait for events:
		numberOfFiredEvents = epoll_wait(polling, firedEvents, 100, -1);
		if (numberOfFiredEvents < 0) {
			logErr("[%s] Error waiting for event: %s", (*this->descriptor).name, strerror(errno));
		} else {
			for (i = 0; i < numberOfFiredEvents; i++) {
				fd = firedEvents[i].data.fd;
				switch(fd) {
				case 0: //*this->messageQueue:
					receiveMessage_BEGIN(this, UserInterface)
						if (result < 0) {
							//TODO Implement appropriate error handling
							sleep(10);

							// Try again
							continue;
						}
						logInfo("[%s] Message received from %s (message length: %u)", (*this->descriptor).name, senderDescriptor.name, result);
						MESSAGE_SELECTOR_BEGIN
						MESSAGE_BY_TYPE_SELECTOR(message, UserInterface, Command)
							logInfo("[%s] User interface command received!", (*this->descriptor).name);
							logInfo("[%s] \tCommand: %u", (*this->descriptor).name, /* message.content.UserInterfaceCommand. */ content.command);
							sendResponse_BEGIN(this, UserInterface, Status)
										.code = 254
							sendResponse_END
						MESSAGE_BY_TYPE_SELECTOR(message, UserInterface, Status)
							logInfo("[%s] User interface status received!");
							logInfo("[%s] \tCode: %u", (*this->descriptor).name, /* message.content.UserInterfaceStatus. */ content.code);
							logInfo("[%s] \tMessage: %s", (*this->descriptor).name, /* message.content.UserInterfaceStatus. */ content.message);
						MESSAGE_SELECTOR_END
					receiveMessage_END
					break;
				case 1: //buttonsFileDescriptor:
					result = read(buttonsFileDescriptor, buffer, 3);
					if (result < 0) {
						logErr("[%s] read button: %s", (*this->descriptor).name, strerror(errno));
					}
					value = atoi(buffer);
					// TODO: Update display and send command
					break;
				case 2: //switchesFileDescriptor:
					result = read(switchesFileDescriptor, buffer, 3);
					if (result < 0) {
						logErr("[%s] read button: %s", (*this->descriptor).name, strerror(errno));
					}
					value = atoi(buffer);
					// TODO: Update display and send command
					break;
				}
			} // foreach event end
		} // if number of fired events end
	}
	close(polling);
	close(buttonsFileDescriptor);
	close(switchesFileDescriptor);
}

static void tearDownUserInterface(void *activity) {
	logInfo("[userInterface] Tearing down...");
	destroyActivity(display);
}
