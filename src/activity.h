/**
 * <short description>
 *
 * @brief   Helper functions to create activities
 * @file    activity.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_

#include <pthread.h>
#include <mqueue.h>
#include <sys/stat.h>

#define MESSAGE_SELECTOR_BEGIN \
	if (0) {
#define MESSAGE_SELECTOR(message, activityName) \
	} else if (strcmp(message.activity.name, #activityName) == 0) {
#define MESSAGE_SELECTOR_ANY \
	} else {
#define MESSAGE_SELECTOR_END \
	}

typedef void (*ActivityRun)(void *activity);

typedef struct {
	unsigned int id;
	char name[30];
	ActivityRun setUp;
	ActivityRun run;
	ActivityRun tearDown;
} ActivityDescriptor;

typedef enum {
	messageQueue_blocking = 0,
	messageQueue_nonBlocking
} MessageQueueMode;

typedef struct {
	ActivityDescriptor *descriptor;
	pthread_t thread;
	mqd_t messageQueue;
	MessageQueueMode messageQueueMode;
} Activity;

typedef enum {
	messagePriority_low = 0,
	messagePriority_medium,
	messagePriority_high,
} MessagePriority;

Activity *createActivity(ActivityDescriptor descriptor, MessageQueueMode messageQueueMode);
void destroyActivity(Activity *activity);

void waitForEvent(Activity *activity, char *buffer, unsigned long length, unsigned int timeout);
long receiveMessage(void *_activity, char *buffer, unsigned long length);
void sendMessage(ActivityDescriptor activity, char *buffer, unsigned long length, MessagePriority priority);

#endif /* ACTIVITY_H_ */
