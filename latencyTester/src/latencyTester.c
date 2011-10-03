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

//#define COFFEE_GRINDER_MOTOR_DEVICE_FILE "/dev/coffeeGrinderMotor"
#define LATENCY_TESTER_DEVICE_FILE "latencyTester"

static pthread_t workerThread;

static int latencyTesterDevice;

//static pthread_cond_t isMessageToWriteConditionQueue;
//static pthread_mutex_t writingMessageMutex;

// Real-time worker thread
static void * runThread(void *argument) {
	//char buffer[256];
	//pthread_mutex_lock(&writingMessageMutex);
	while (TRUE) {
		read(latencyTesterDevice, NULL, 0);
		write(latencyTesterDevice, NULL, 0);
	}

	return NULL;
}

static int isSetUp = FALSE;

int setUpLatencyTester() {
	int result = 0;

	logInfo("[latencyTester] Setting up...");

	/*
	pthread_mutexattr_t mutexAttributes;
	pthread_mutexattr_init(&mutexAttributes);
	pthread_mutexattr_setprotocol(&mutexAttributes, PTHREAD_PRIO_INHERIT);
	if ((result = pthread_mutex_init(&writingMessageMutex, &mutexAttributes))) {
		logErr("[latencyTester] Error initializing mutex: %s", strerror(result));

		goto setUpLatencyTester_out;
	}

	pthread_condattr_t conditionQueueAttributes;
	pthread_condattr_init(&conditionQueueAttributes);
	pthread_condattr_setpshared(&conditionQueueAttributes, PTHREAD_PROCESS_SHARED);
	if ((result = pthread_cond_init(&isMessageToWriteConditionQueue, &conditionQueueAttributes))) {
		logErr("[latencyTester] Error initializing condition queue: %s", strerror(result));

		goto setUpLatencyTester_out;
	}
	*/
	if ((latencyTesterDevice = open(LATENCY_TESTER_DEVICE_FILE, O_RDWR)) >= 0) {
		logInfo("[latencyTester] LatencyTester device opened (file descriptor: %d)", latencyTesterDevice);
	}

	pthread_attr_t threadAttributes;
	pthread_attr_init(&threadAttributes);
	pthread_attr_setinheritsched(&threadAttributes, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&threadAttributes, SCHED_FIFO);
	struct sched_param schedulingParameter;
	schedulingParameter.__sched_priority = 99;
	pthread_attr_setschedparam(&threadAttributes, &schedulingParameter);

	if ((result = pthread_create(&workerThread, &threadAttributes, runThread, NULL))) {
		logErr("[latencyTester] Error creating worker thread: %s", strerror(result));

		goto setUpLatencyTester_out;
	}
#ifdef __XENO__
    pthread_set_name_np(workerThread, "worker");
#endif

	//if (!writeNonBlockingDevice(COFFEE_GRINDER_MOTOR_DEVICE_FILE, "20", wrm_replace, FALSE)) {
	//}

	isSetUp = TRUE;

setUpLatencyTester_out:
	return result;
}

void tearDownLatencyTester() {
	if (!isSetUp) {
		return;
	}

	logInfo("[latencyTester] Tearing down...");

	//if (!writeNonBlockingDevice(COFFEE_GRINDER_MOTOR_DEVICE_FILE, "0", wrm_replace, FALSE)) {
	//	logErr("[latencyTester] Error stopping motor!");
	//}

	pthread_cancel(workerThread);
	pthread_join(workerThread, NULL);

	//pthread_cond_destroy(&isMessageToWriteConditionQueue);
	//pthread_mutex_destroy(&writingMessageMutex);

	close(latencyTesterDevice);
}

static void handleCtrlC(int sig) {
	// Do nothing here
}

int main(int argc, char **argv) {
	int result = 0;

	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (setUpLatencyTester()) {
			result = 1;
			goto main_out;
	}

	// Establish the signal handler
	(void) signal(SIGINT, handleCtrlC);

	// Wait for SIGINT signal
	pause();

	tearDownLatencyTester();

main_out:
	return result;
}

