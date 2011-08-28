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

typedef void (*ActivityRun)(void *activity);

typedef struct {
	char *name;
	ActivityRun setUp;
	ActivityRun run;
	ActivityRun tearDown;
} ActivityDescriptor;

typedef enum {
	mq_blockable = 0,
	mq_nonblockable
} MqMode;

typedef struct {
	ActivityDescriptor *descriptor;
	pthread_t thread;
	mqd_t messageQueue;
	MqMode mqMode;
} Activity;

typedef enum {
	prio_low = 0,
	prio_medium,
	prio_high,
} MessagePrio;

Activity *createActivity(ActivityDescriptor descriptor, MqMode mqMode);
void destroyActivity(Activity *activity);

unsigned long receiveMessage(void *_activity, char *buffer, unsigned long length);
void sendMessage(ActivityDescriptor activity, char *buffer, unsigned long length, MessagePrio prio);

#endif /* ACTIVITY_H_ */
