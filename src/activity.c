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
#include "activity.h"

static void logError(char *message) {
	perror(message);
	printf("Log: [ERROR]: %s\n", message);
}

static char *createMessageQueueId(char *activityName) {
	size_t idLength = strlen(activityName) + 2;
	char *id = (char *) malloc(idLength);
	memset(id, 0, idLength);
	id[0] = '/';
	strcat(id, activityName);

	return id;
}

static mqd_t createMessageQueue(char *activityName) {
	char *id = createMessageQueueId(activityName);

	struct mq_attr attributes = {
			.mq_maxmsg = 10,
			.mq_msgsize = 10
	};

	mqd_t queue = mq_open(id, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG, &attributes);
	free(id);
	if (queue < 0) {
		logError("Error creating queue!");
	}

	return queue;
}

static void * runThread(void *argument) {
	Activity *activity = (Activity *)argument;

	pthread_cleanup_push(activity->descriptor->tearDown, NULL);

	activity->descriptor->setUp(NULL);

	activity->descriptor->run(activity);

	printf("Thread terminated.\n");

	pthread_cleanup_pop(1);

	return NULL;
}

Activity *createActivity(ActivityDescriptor descriptor) {
	// Create new activity instance
	Activity *activity = (Activity *) malloc(sizeof(Activity));
	memset(activity, 0, sizeof(Activity));

	// Copy activity descriptor (from stack to heap)
	ActivityDescriptor *descriptorCopy = (ActivityDescriptor *)malloc(sizeof(ActivityDescriptor));
	memcpy(descriptorCopy, &descriptor, sizeof(ActivityDescriptor));
	activity->descriptor = descriptorCopy;

	// Create new message queue (= queue for incoming messages)
	mqd_t messageQueue = createMessageQueue(descriptor.name);
	if (messageQueue < 0) {
		logError("Error creating activity's message queue for incoming messages!");

		// TODO Error handling
	}
	activity->messageQueue = messageQueue;

	// Start new thread
	pthread_t thread;
	if (pthread_create(&thread, NULL, runThread, activity) < 0) {
		logError("Error creating new thread for activity!");

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
	mq_unlink(messageQueueId);
	free(messageQueueId);

	free(activity->descriptor);
	free(activity);
}

unsigned long receiveMessage(void *_activity, char *buffer, unsigned long length) {
	Activity *activity = (Activity *)_activity;

	char receiveBuffer[11];
	ssize_t n;
	if ((n = mq_receive(activity->messageQueue, receiveBuffer, sizeof(receiveBuffer), NULL)) < 0) {
		logError("Error receiving message!");
	}
	if (n != length) {

	}
	memcpy(buffer, receiveBuffer, length);

	return n;
}

void sendMessage(ActivityDescriptor activity, char *buffer, unsigned long length) {
	char *messageQueueId = createMessageQueueId(activity.name);
	mqd_t queue = mq_open(messageQueueId, O_WRONLY);
	free(messageQueueId);
	if (queue < 0) {
		logError("Error opening message queue for sending!");

		return;
	}

	if (mq_send(queue, buffer, length, /* priority: */ 1) < 0) {
		logError("Error sending message!");
	}
	mq_close(queue);
}
