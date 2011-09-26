/**
 * @brief   Submodule RT-model display
 * @file    rtModelDisplay.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 22, 2011
 */

#include <unistd.h>
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

static void setUpRtModelDisplay(void *activity) {
	logInfo("[rtModelDisplay] Setting up...");

	this = (Activity *)activity;
}

static void runRtModelDisplay(void *activity) {
	logInfo("[rtModelDisplay] Running up...");

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
}
