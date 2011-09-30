/**
 * @brief   User space testing programm
 * @file    latencyTester.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 30, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <defines.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <device.h>
#include <log.h>

#define COFFEE_GRINDER_MOTOR_DEVICE_FILE "/dev/coffeeGrinderMotor"

static pthread_t workerThread;

static char *message = 0;
static volatile char *characterToWrite = 0;

static int displayDevice;

static pthread_cond_t isMessageToWriteConditionQueue;
static pthread_mutex_t writingMessageMutex;

// Real-time worker thread
static void * runThread(void *argument) {
	struct timespec time;
	pthread_mutex_lock(&writingMessageMutex);
	while (TRUE) {
		read(displayDevice, NULL, 0);
		write(displayDevice, NULL, 0);
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

	pthread_condattr_t conditionQueueAttributes;
	pthread_condattr_init(&conditionQueueAttributes);
	pthread_condattr_setpshared(&conditionQueueAttributes, PTHREAD_PROCESS_SHARED);
	if ((result = pthread_cond_init(&isMessageToWriteConditionQueue, &conditionQueueAttributes))) {
		logErr("[display] Error initializing condition queue: %s", strerror(result));

		goto setUpDisplay_out;
	}

	if ((displayDevice = open("rt-model-display", O_RDONLY)) >= 0) {
		logInfo("[display] Display device opened (file descriptor: %d)", displayDevice);
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

	logInfo("Triggering worker thread...");

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

static void handleCtrlC(int sig)
{
	// Do nothing here
}

int main(int argc, char **argv) {
	int result = 0;

	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (setUpDisplay()) {
			result = 1;

			goto main_out;
		}

		writeDisplay(argv[1]);
		joinDisplay();

		tearDownDisplay();
	}
main_out:
	return result;
}

