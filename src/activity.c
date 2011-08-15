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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include "activity.h"

static mqd_t createMessageQueue(char *id) {
	char *idStr;
	struct mq_attr attributes;
	mqd_t queue;
	idStr = (char *) malloc(strlen(id)+2);
	memset(idStr, 0, strlen(id)+2);
	idStr[0] = '/';
	strcat(idStr, id);
	queue = mq_open(idStr, O_CREAT, S_IRWXU | S_IRWXG, &attributes);
	free(idStr);
	if (queue < 0) {
		logError("Error creating queue");
	}
	return queue;
}

Activity *createActivity(ActivityRun run) {
	Activity *activity = (Activity *) malloc(sizeof(pthread_t));
	memset(activity, 0, sizeof(Activity));
	pthread_create(activity, NULL, run, NULL);
	createMessageQueue();
	return (activity);
}

void destroyActivity(Activity *activity) {
	pthread_cancel(activity);
	pthread_join(activity, NULL);
	free(activity);
}

