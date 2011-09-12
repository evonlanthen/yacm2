/**
 * @brief   Helper functions to create activities
 * @file    activity.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include "syslog.h"
#include "activity.h"

#define MAX_MESSAGE_LENGTH 400

static char *createMessageQueueId(char *activityName) {
	size_t idLength = strlen(activityName) + 2;
	char *id = (char *) malloc(idLength);
	memset(id, 0, idLength);
	id[0] = '/';
	strcat(id, activityName);

	return id;
}

static mqd_t createMessageQueue(char *activityName, MessageQueueMode messageQueueMode) {
	char *id = createMessageQueueId(activityName);

	if (mq_unlink(id) < 0) {
		// Ignore any errors
	}

	struct mq_attr attributes = {
			.mq_maxmsg = 10,
			.mq_msgsize = MAX_MESSAGE_LENGTH
	};

	mqd_t queue;
	if (messageQueueMode == messageQueue_nonBlocking) {
		queue = mq_open(id, O_CREAT | O_RDONLY | O_NONBLOCK, S_IRWXU | S_IRWXG, &attributes);
	} else {
		queue = mq_open(id, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG, &attributes);
	}

	//
	free(id);
	if (queue < 0) {
		logErr("[%s] Error creating message queue %s: %s", activityName, id, strerror(errno));
	}

	return queue;
}

static void * runThread(void *argument) {
	Activity *activity = (Activity *)argument;

	pthread_cleanup_push(activity->descriptor->tearDown, NULL);

	activity->descriptor->setUp(NULL);

	activity->descriptor->run(activity);

	logInfo("[activity] Thread terminated.");

	pthread_cleanup_pop(1);

	return NULL;
}

Activity *createActivity(ActivityDescriptor descriptor, MessageQueueMode messageQueueMode) {
	// Create new activity instance
	Activity *activity = (Activity *) malloc(sizeof(Activity));
	memset(activity, 0, sizeof(Activity));

	// Copy activity descriptor (from stack to heap)
	ActivityDescriptor *descriptorCopy = (ActivityDescriptor *)malloc(sizeof(ActivityDescriptor));
	memcpy(descriptorCopy, &descriptor, sizeof(ActivityDescriptor));
	activity->descriptor = descriptorCopy;

	// Create new message queue (= queue for incoming messages)
	mqd_t messageQueue = createMessageQueue(descriptor.name, messageQueueMode);
	if (messageQueue < 0) {
		logErr("[%s] Error creating activity's message queue for incoming messages: %s", descriptor.name, strerror(errno));

		// TODO Error handling
	}
	activity->messageQueue = messageQueue;
	activity->messageQueueMode = messageQueueMode;

	// Start new thread
	pthread_t thread;
	if (pthread_create(&thread, NULL, runThread, activity) < 0) {
		logErr("[%s] Error creating new thread for activity: %s", descriptor.name, strerror(errno));
		// TODO Error handling
	}
	activity->thread = thread;

	return activity;
}

void destroyActivity(Activity *activity) {
	pthread_cancel(activity->thread);
	pthread_join(activity->thread, NULL);

	char *messageQueueId = createMessageQueueId(activity->descriptor->name);
	mq_close(activity->messageQueue);
	if (mq_unlink(messageQueueId) < 0) {
		logErr("[%s] Error destroying activity's message queue for incoming messages: %s", activity->descriptor->name, strerror(errno));
	}
	free(messageQueueId);

	free(activity->descriptor);
	free(activity);
}

int waitForEvent(Activity *activity, char *buffer, unsigned long length, unsigned int timeout) {
	logInfo("[%s] Going to wait for an event...", activity->descriptor->name);

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
		logInfo("[%s] Message received!", activity->descriptor->name);

		unsigned long incomingMessageLength = receiveMessage(activity, buffer, length);

		/*
		from %s (%d): intVal=%d, strVal='%s'",
		activity->descriptor.name,
		buffer->activity.name,
		incomingMessageLength,
		buffer->intValue,
		buffer->strValue);
		*/

		return incomingMessageLength;
	} else {
		logInfo("[%s] Timeout occured!", activity->descriptor->name);

		return 0;
	}
}

int receiveMessage(void *_activity, char *buffer, unsigned long length) {
	if (!_activity) {
		logErr("["__FILE__"] null pointer at receiveMessage(_activity, ...)");

		return -EFAULT;
	}

	Activity *activity = (Activity *)_activity;

	char receiveBuffer[MAX_MESSAGE_LENGTH + 1];
	ssize_t messageLength;
	if ((messageLength = mq_receive(activity->messageQueue, receiveBuffer, sizeof(receiveBuffer), NULL)) < 0) {
		if (activity->messageQueueMode != messageQueue_nonBlocking) {
			logErr("[%s] Error receiving message: %s", activity->descriptor->name, strerror(errno));
		}
	}
	if (messageLength > length) {
		return -EFAULT;
	}
	memcpy(buffer, receiveBuffer, messageLength);

	return messageLength;
}

int sendMessage(ActivityDescriptor receiverDescriptor, char *buffer, unsigned long length, MessagePriority priority) {
	if (length > MAX_MESSAGE_LENGTH) {
		return -EFAULT;
	}

	char *receiverMessageQueueId = createMessageQueueId(receiverDescriptor.name);

	mqd_t receiverQueue = mq_open(receiverMessageQueueId, O_WRONLY);
	free(receiverMessageQueueId);
	if (receiverQueue < 0) {
		logErr("[%s] Error opening message queue for sending: %s", "<Sender>", strerror(errno));

		return -EFAULT;
	}

	//logInfo("[%s] Sending message (message length: %u)...", "<Sender>", length);

	if (mq_send(receiverQueue, buffer, length, priority) < 0) {
		logErr("[%s] Error sending message: %s", "<Sender>", strerror(errno));

		return -EFAULT;
	}

	mq_close(receiverQueue);

	return 0;
}
