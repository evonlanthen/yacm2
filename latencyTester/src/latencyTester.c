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

#define LATENCY_TESTER_DEVICE_FILE "latencyTester"

static pthread_t workerThread;
static int latencyTesterDevice;

// Real-time worker thread
static void * runThread(void *argument) {
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

	isSetUp = TRUE;

setUpLatencyTester_out:
	return result;
}

void tearDownLatencyTester() {
	if (!isSetUp) {
		return;
	}

	logInfo("[latencyTester] Tearing down...");

	pthread_cancel(workerThread);
	pthread_join(workerThread, NULL);

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

