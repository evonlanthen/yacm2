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
#include <fcntl.h>
#include <pthread.h>
//#include <native/cond.h>
#include <time.h>
#include <defines.h>
#include <device.h>
#include <log.h>

#define COFFEE_GRINDER_MOTOR_DEVICE_FILE "/dev/coffeeGrinderMotor"

static pthread_t workerThread;

static char *message = 0;
static volatile char *characterToWrite = 0;

static int displayDevice;

static pthread_cond_t isMessageToWriteConditionQueue;
static pthread_mutex_t writingMessageMutex;

//static RT_COND c;
//static RT_MUTEX m;

// Real-time worker thread
static void * runThread(void *argument) {
	struct timespec time;
//	struct timespec previousTime = {
//			.tv_nsec = 0,
//			.tv_sec = 0
//	};
//	unsigned long long elapsedTime;
	pthread_mutex_lock(&writingMessageMutex);
//	rt_mutex_acquire(&m, TM_INFINITE);
//	struct timespec time = {
//			.tv_sec = 1,
//			.tv_nsec = 0
//	};
//	nanosleep(&time, NULL);
	while (TRUE) {
		//printf("***\n");
		int result;
		if ((result = pthread_cond_wait(&isMessageToWriteConditionQueue, &writingMessageMutex))) {
			logErr("[display] Error waiting for message: %s", strerror(result));

			//usleep(10000);
		}
//		if ((result = rt_cond_wait(&c, &m, TM_INFINITE)) < 0) {
//			logErr("[display] Error waiting for message: %s", strerror(result));
//		}

		printf("Going to write message...\n");
		while (characterToWrite && *characterToWrite) {
			printf("%c\n", *characterToWrite);

			//int i;
			//for (i = 0; i < 50; i++) {

				//logInfo("%c", *characterToWrite);
				//printf("%c\n", *characterToWrite);
				//read(displayDevice, NULL, 0);
				clock_gettime(CLOCK_MONOTONIC, &time);
				printf("time.tv_sec: %ld\n", time.tv_sec);
				printf("time.tv_nsec: %ld\n", time.tv_nsec);

				//write(displayDevice, NULL, 0);
			//}

			characterToWrite++;
		}

//		struct timespec time;
//		time.tv_sec = 1;
//		nanosleep(&time, NULL);
	}

	return NULL;
}

static int isSetUp = FALSE;

int setUpDisplay() {
	int result = 0;

	logInfo("[display] Setting up...");

	pthread_mutexattr_t mutexAttributes;
	pthread_mutexattr_init(&mutexAttributes);
	pthread_mutexattr_setprotocol(&mutexAttributes, PTHREAD_PRIO_INHERIT);
	if ((result = pthread_mutex_init(&writingMessageMutex, &mutexAttributes))) {
		logErr("[display] Error initializing mutex: %s", strerror(result));

		goto setUpDisplay_out;
	}

//	rt_mutex_create(&m, "m");

	if ((result = pthread_cond_init(&isMessageToWriteConditionQueue, NULL))) {
		logErr("[display] Error initializing condition queue: %s", strerror(result));

		goto setUpDisplay_out;
	}

//	rt_cond_create(&c, "c");

	if ((displayDevice = open("rt-model-display", O_RDONLY)) >= 0) {
		logInfo("[display] Display device opened (file descriptor: %d)", displayDevice);
	} else {
		//logErr("[display] Error opening display device: %s", strerror(errno));
		//result = 1;

		//goto setUpDisplay_out;
	}

	pthread_attr_t threadAttributes;
	pthread_attr_init(&threadAttributes);
	pthread_attr_setinheritsched(&threadAttributes, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&threadAttributes, SCHED_FIFO);
	struct sched_param schedulingParameter;
	schedulingParameter.__sched_priority = 99;
	pthread_attr_setschedparam(&threadAttributes, &schedulingParameter);

	if ((result = pthread_create(&workerThread, &threadAttributes, runThread, NULL))) {
		logErr("[display] Error creating worker thread: %s", strerror(result));

		goto setUpDisplay_out;
	}
#ifdef __XENO__
    pthread_set_name_np(workerThread, "worker");
#endif

	if (!writeNonBlockingDevice(COFFEE_GRINDER_MOTOR_DEVICE_FILE, "20", wrm_replace, FALSE)) {
		//logErr("[display] Error starting motor!");
		//result = 1;

		//goto setUpDisplay_out;
	}

	isSetUp = TRUE;

setUpDisplay_out:
	return result;
}

void writeDisplay(char *newMessage) {
	if (!isSetUp) {
		logErr("[display] Invalid operation!");

		return;
	}

	printf(">>> %s <<<\n", newMessage);

//	pthread_mutex_lock(&writingMessageMutex);

	logInfo("Triggering worker thread...");

	//TODO Add synchronization with (real-time) worker thread
	if (message) {
		free(message);
	}
	size_t newMessageLength = strlen(newMessage);
	if ((message = malloc(newMessageLength + 1))) {
		memcpy(message, newMessage, newMessageLength + 1);
		characterToWrite = message;

		//pthread_mutex_unlock(&writingMessageMutex);
//		int result;
//		if ((result = pthread_cond_signal(&isMessageToWriteConditionQueue))) {
//			logErr("[display] Error signaling worker thread: %s", strerror(result));
//		}
	} else {
		logErr("[display] Error copying message: %s", strerror(errno));
	}

//	pthread_mutex_unlock(&writingMessageMutex);
}

void joinDisplay() {
	if (!isSetUp) {
		logErr("[display] Invalid operation!");

		return;
	}

	//TODO Improve synchronization
	while (characterToWrite && *characterToWrite) {
		usleep(1000);
	}
}

void tearDownDisplay() {
	if (!isSetUp) {
		return;
	}

	logInfo("[display] Tearing down...");

	if (!writeNonBlockingDevice(COFFEE_GRINDER_MOTOR_DEVICE_FILE, "0", wrm_replace, FALSE)) {
		logErr("[display] Error stopping motor!");
	}

	pthread_cancel(workerThread);
	pthread_join(workerThread, NULL);

	pthread_cond_destroy(&isMessageToWriteConditionQueue);
	pthread_mutex_destroy(&writingMessageMutex);

	close(displayDevice);

	if (message) {
		free(message);
	}
}
