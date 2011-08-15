/**
 * <short description>
 *
 * <long description>
 *
 * @file    activity.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_

#include <pthread.h>
#include <mqueue.h>

typedef void (*ActivityRun)(void *activity);

typedef struct {
	char *name;
	ActivityRun setUp;
	ActivityRun run;
	ActivityRun tearDown;
} ActivityDescriptor;

typedef struct {
	ActivityDescriptor *descriptor;
	pthread_t thread;
	mqd_t messageQueue;
} Activity;

Activity *createActivity(ActivityDescriptor descriptor);
void destroyActivity(Activity *activity);

unsigned long receiveMessage(void *_activity, char *buffer, unsigned long length);
void sendMessage(ActivityDescriptor activity, char *buffer, unsigned long length);

#endif /* ACTIVITY_H_ */
