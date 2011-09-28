/**
 * @brief   Display logic
 * @file    display.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 22, 2011
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <defines.h>
#include <log.h>

static pthread_t thread;

static char *message = 0;
static volatile char *characterToWrite = 0;

// Real-time thread
static void * runThread(void *argument) {
	while (TRUE) {
		//logInfo("[rtModelDisplay] Heartbeat!");
		if (characterToWrite && *characterToWrite) {
			//logInfo("%c", *characterToWrite);
			//printf("%c\n", *characterToWrite);

			characterToWrite++;
		}

		struct timespec time;
		time.tv_sec = 1;
		nanosleep(&time, NULL);
	}

	return NULL;
}

void setUpDisplay() {
	logInfo("[display] Setting up...");

	pthread_attr_t threadAttributes;
	pthread_attr_init(&threadAttributes);
	pthread_attr_setinheritsched(&threadAttributes, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&threadAttributes, SCHED_FIFO);
	struct sched_param schedulingParameter;
	schedulingParameter.__sched_priority = 50;
	pthread_attr_setschedparam(&threadAttributes, &schedulingParameter);

	if (pthread_create(&thread, &threadAttributes, runThread, NULL) < 0) {
		logErr("[display] Error creating new thread: %s", strerror(errno));
	}
}

void writeDisplay(char *newMessage) {
	printf(">>> %s <<<\n", newMessage);

	//TODO Add synchronization with real-time thread
	if (message) {
		free(message);
	}
	size_t newMessageLength = strlen(newMessage);
	if ((message = malloc(newMessageLength + 1))) {
		memcpy(message, newMessage, newMessageLength + 1);
		characterToWrite = message;
	} else {
		logErr("[display] Error copying new message: %s", strerror(errno));
	}
}

void joinDisplay() {
	//TODO Improve synchronization
	while (characterToWrite && *characterToWrite) {
		usleep(1000);
	}
}

void tearDownDisplay() {
	logInfo("[display] Tearing down...");

	pthread_cancel(thread);
	pthread_join(thread, NULL);

	if (message) {
		free(message);
	}
}
