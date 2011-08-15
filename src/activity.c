/**
 * <short description>
 *
 * <long description>
 *
 * @file    activity.c
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "activity.h"

static void logError(char *message) {
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

	mqd_t queue = mq_open(id, O_CREAT, S_IRWXU | S_IRWXG, &attributes);
	free(id);
	if (queue < 0) {
		logError("Error creating queue!");
	}

	return queue;
}

static void * runThread(void *par) {
	ActivityDescriptor *activityDescriptor = (ActivityDescriptor *)par;

	pthread_cleanup_push(activityDescriptor->tearDown, NULL);

	activityDescriptor->setUp(NULL);

	activityDescriptor->run(NULL);

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

	// Start new thread
	pthread_t thread;
	if (pthread_create(&thread, NULL, runThread, descriptorCopy) < 0) {
		logError("Error creating new thread for activity!");
	}
	activity->thread = thread;

	// Create new message queue (= queue for incoming messages)
	mqd_t messageQueue = createMessageQueue(descriptor.name);
	if (messageQueue < 0) {
		logError("Error creating activity's message queue for incoming messages!");
	}
	activity->messageQueue = messageQueue;

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

