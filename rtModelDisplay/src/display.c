/**
 * @brief   Display logic
 * @file    display.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 22, 2011
 */

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <native/event.h>
#include <time.h>
#include <defines.h>
#include <device.h>
#include <log.h>

#define NANOSECONDS_PER_SECOND 1000000000

//#define NO_DISPLAY
#define APPLICATION_LOGIC
//#define DEVICE_DRIVER_LOGIC

#define COFFEE_GRINDER_MOTOR_DEVICE_FILE "/dev/coffeeGrinderMotor"

#define BLANK_IMAGE_INDEX 35
#define UNDEFINED_CHARACTER_IMAGE_INDEX 0

static pthread_t workerThread;

static char *messageBuffer = 0;
static volatile int newMessage = 0;

static int displayDevice;

static pthread_mutex_t writingDisplayMutex;
static pthread_mutex_t writingMessageMutex;
//static pthread_cond_t isMessageToWriteConditionQueue;

static RT_EVENT events;

//static RT_COND c;
//static RT_MUTEX m;

// Real-time worker thread
static void * runThread(void *argument) {
	struct timespec blankTime = {
			.tv_sec = 0,
			.tv_nsec = NANOSECONDS_PER_SECOND / 2 / 5
	};
	struct timespec showCharacterTime = {
			.tv_sec = 0,
			.tv_nsec = NANOSECONDS_PER_SECOND / 2
	};

	struct timespec previousTime = {
			.tv_nsec = 0,
			.tv_sec = 0
	};

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while (TRUE) {
		int result;
//		if ((result = pthread_mutex_lock(&writingMessageMutex))) {
//			logErr("[display] Error locking mutex: %s", strerror(result));
//		}
		unsigned long raisedEvents;
		if ((result = rt_event_wait(&events, 2, &raisedEvents, EV_ALL, TM_INFINITE)) < 0) {
			logErr("[display] Error waiting for event: %s", strerror(result));
		}
		rt_event_clear(&events, 2, &raisedEvents);

//		if ((result = pthread_cond_wait(&isMessageToWriteConditionQueue, &writingMessageMutex))) {
//			logErr("[display] Error waiting for message: %s", strerror(result));
//
//			//usleep(10000);
//		}

		printf("Going to write message...\n");

		pthread_mutex_lock(&writingDisplayMutex);
		newMessage = FALSE;
		char *c = messageBuffer;
		pthread_mutex_unlock(&writingDisplayMutex);

		int abort = FALSE;

		int isCharacter = 0;
		while (TRUE) {
			pthread_mutex_lock(&writingDisplayMutex);
			if (newMessage) {
				abort = TRUE;
			}
			pthread_mutex_unlock(&writingDisplayMutex);

			// If aborted or finished...
			if (abort || !(c && *c)) {
				break;
			}

#if defined(APPLICATION_LOGIC) || defined(DEVICE_DRIVER_LOGIC)
			signed char imageIndex = 0;
			if (isCharacter) {
				if (*c == ' ') {
					imageIndex = BLANK_IMAGE_INDEX;
				} else if (*c == '.') {
					imageIndex = 30;
				} else if ((*c >= 'A' && *c <= 'Z')
					|| (*c >= 'a' && *c <= 'z')) {
					imageIndex = toupper(*c) - 'A' + 1;
				} else {
					imageIndex = UNDEFINED_CHARACTER_IMAGE_INDEX; // Image for undefined character
				}
			} else {
				imageIndex = BLANK_IMAGE_INDEX;
			}

			imageIndex = imageIndex - 2;
			if (imageIndex < 0) {
				imageIndex = 36 + imageIndex;
			}
#endif

#if defined(NO_DISPLAY)
			if (isCharacter) {
				printf("%c", *c);
				fflush(stdout);

				nanosleep(&showCharacterTime, NULL);
			} else {
				printf(" ");
				fflush(stdout);

				nanosleep(&blankTime, NULL);
			}
#elif defined(APPLICATION_LOGIC)
			int i;
			for (i = 0; i < (isCharacter ? 50 : 10); i++) {
				read(displayDevice, NULL, 0);

				struct timespec time;
				clock_gettime(CLOCK_MONOTONIC, &time);
				//printf("time.tv_sec: %ld\n", time.tv_sec);
				//printf("time.tv_nsec: %ld\n", time.tv_nsec);

				if (previousTime.tv_sec > 0) {
					unsigned long long elapsedTime = (time.tv_sec - previousTime.tv_sec) * NANOSECONDS_PER_SECOND + (time.tv_nsec - previousTime.tv_nsec);
					//printf("elapsedTime: %lld\n", elapsedTime);

					long d = elapsedTime / 36 * imageIndex - 100000;
					struct timespec delayTime = {
							.tv_nsec = d,
							.tv_sec = 0
					};
					nanosleep(&delayTime, NULL);

					write(displayDevice, NULL, 0);
				}

				previousTime.tv_sec = time.tv_sec;
				previousTime.tv_nsec = time.tv_nsec;
			}
#elif defined(DEVICE_DRIVER_LOGIC)
			write(displayDevice, imageIndex, 1);

			if (isCharacter) {
				nanosleep(&showCharacterTime, NULL);
			} else {
				nanosleep(&blankTime, NULL);
			}
#endif

			if (isCharacter) {
				c++;
			}

			isCharacter = ++isCharacter % 2;
		}

		if (!abort) {
			if ((result = rt_event_signal(&events, 1)) < 0) {
				logErr("[display] Error signaling event: %s", strerror(result));
			}
		}
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

	if ((result = pthread_mutex_init(&writingDisplayMutex, &mutexAttributes))) {
		logErr("[display] Error initializing mutex: %s", strerror(result));

		goto setUpDisplay_out;
	}

	if ((result = pthread_mutex_init(&writingMessageMutex, &mutexAttributes))) {
		logErr("[display] Error initializing mutex: %s", strerror(result));

		goto setUpDisplay_out;
	}
	pthread_mutex_lock(&writingMessageMutex);

	rt_event_create(&events, "events", 0, EV_PRIO);

//	pthread_condattr_t conditionQueueAttributes;
//	pthread_condattr_init(&conditionQueueAttributes);
//	pthread_condattr_setpshared(&conditionQueueAttributes, PTHREAD_PROCESS_SHARED);
//	if ((result = pthread_cond_init(&isMessageToWriteConditionQueue, &conditionQueueAttributes))) {
//		logErr("[display] Error initializing condition queue: %s", strerror(result));
//
//		goto setUpDisplay_out;
//	}

	if ((displayDevice = open("rt-model-display", O_RDONLY)) >= 0) {
		logInfo("[display] Display device opened (file descriptor: %d)", displayDevice);
	} else {
		logErr("[display] Error opening display device: %s", strerror(errno));
		result = 1;

		goto setUpDisplay_out;
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

	if (!writeNonBlockingDevice(COFFEE_GRINDER_MOTOR_DEVICE_FILE, "50", wrm_replace, FALSE)) {
		logErr("[display] Error starting motor!");
		result = 1;

		goto setUpDisplay_out;
	}

	sleep(1);

	isSetUp = TRUE;

setUpDisplay_out:
	return result;
}

void writeDisplay(char *message) {
	if (!isSetUp) {
		logErr("[display] Invalid operation!");

		return;
	}

	logInfo("Message: %s", message);

	pthread_mutex_lock(&writingDisplayMutex);
	unsigned long raisedEvents;
	rt_event_clear(&events, 1, &raisedEvents);

	logInfo("Triggering worker thread...");

	//TODO Add synchronization with (real-time) worker thread
	if (messageBuffer) {
		free(messageBuffer);
	}
	size_t messageLength = strlen(message);
	if ((messageBuffer = malloc(messageLength + 1))) {
		memset(messageBuffer, 0, messageLength + 1);
		memcpy(messageBuffer, message, messageLength + 1);

		newMessage = TRUE;

		int result;
//		if ((result = pthread_mutex_unlock(&writingMessageMutex))) {
//			logErr("[display] Error unlocking mutex: %s", strerror(result));
//		}
//		if ((result = pthread_cond_signal(&isMessageToWriteConditionQueue))) {
//			logErr("[display] Error signaling worker thread: %s", strerror(result));
//		}
		if ((result = rt_event_signal(&events, 2)) < 0) {
			logErr("[display] Error signaling event: %s", strerror(result));
		}
	} else {
		logErr("[display] Error copying message: %s", strerror(errno));
	}

	pthread_mutex_unlock(&writingDisplayMutex);
}

void joinDisplay() {
	if (!isSetUp) {
		logErr("[display] Invalid operation!");

		return;
	}

	unsigned long raisedEvents;
	int result;
	if ((result = rt_event_wait(&events, 1, &raisedEvents, EV_ALL, TM_INFINITE)) < 0) {
		logErr("[display] Error waiting for event: %s", strerror(result));
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

	pthread_mutex_destroy(&writingDisplayMutex);
	pthread_mutex_destroy(&writingMessageMutex);
//	pthread_cond_destroy(&isMessageToWriteConditionQueue);

	rt_event_delete(&events);

	close(displayDevice);

	if (messageBuffer) {
		free(messageBuffer);
	}
}
