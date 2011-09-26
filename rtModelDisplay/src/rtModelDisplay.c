/**
 * @brief   Submodule RT-model display
 * @file    rtModelDisplay.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 22, 2011
 */

#include <unistd.h>
#include <time.h>
#include <defines.h>
#include <log.h>
#include "display.h"
#include "rtModelDisplay.h"
#include "rtModelDisplayMessageMapping.h"

// Activity handler prototypes
static void setUpRtModelDisplay(void *activity);
static void runRtModelDisplay(void *activity);
static void tearDownRtModelDisplay(void *activity);

// Activity descriptor
static ActivityDescriptor rtModelDisplay = {
	.name = "rtModelDisplay",
	.setUp = setUpRtModelDisplay,
	.run = runRtModelDisplay,
	.tearDown = tearDownRtModelDisplay
};

static Activity *this;

//static ActivityDescriptor clientDescriptor;

ActivityDescriptor getRtModelDisplayDescriptor() {
	return rtModelDisplay;
}

static pthread_t thread;

static void * runThread() {
	while (TRUE) {
		//logInfo("[rtModelDisplay] Heartbeat!");

		struct timespec time;
		time.tv_sec = 1;
		nanosleep(&time, NULL);
	}

	return NULL;
}

static void setUpRtModelDisplay(void *activity) {
	logInfo("[rtModelDisplay] Setting up...");

	this = (Activity *)activity;

	pthread_attr_t threadAttributes;
	pthread_attr_init(&threadAttributes);
	pthread_attr_setinheritsched(&threadAttributes, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&threadAttributes, SCHED_FIFO);
	struct sched_param par;
	par.__sched_priority = 50;
	pthread_attr_setschedparam(&threadAttributes, &par);

	if (pthread_create(&thread, &threadAttributes, runThread, activity) < 0) {
		logErr("[rtModelDisplay] Error creating new thread: %s", strerror(errno));
	}
}

static void runRtModelDisplay(void *activity) {
	logInfo("[rtModelDisplay] Running...");

	while (TRUE) {
		receiveMessage_BEGIN(this, RtModelDisplay)
			if (error) {
				//TODO Implement appropriate error handling
				sleep(10);

				// Try again
				continue;
			}
			if (result > 0) {
				//TODO Implement business logic
				MESSAGE_SELECTOR_BEGIN
					MESSAGE_BY_TYPE_SELECTOR(message, RtModelDisplay, ShowMessageCommand)
						writeDisplay(content.message);
				MESSAGE_SELECTOR_END
			}
		receiveMessage_END
	}
}

static void tearDownRtModelDisplay(void *activity) {
	logInfo("[rtModelDisplay] Tearing down...");

	pthread_cancel(thread);
	pthread_join(thread, NULL);
}
