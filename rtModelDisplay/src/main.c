/**
 * @brief   Main
 * @file    main.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 22, 2011
 */

#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <defines.h>
#include <activity.h>
#include "rtModelDisplay.h"
#include "display.h"
#include "log.h"

static void handleCtrlC(int sig)
{
	// Do nothing here
}

int main(int argc, char **argv) {
	int result = 0;

	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (argc > 1) {
		if (setUpDisplay()) {
			result = 1;

			goto main_out;
		}

		sleep(1);

		logInfo("Going to write message to display...");
		writeDisplay(argv[1]);

//		sleep(3);
//
//		writeDisplay("Bye!");
//
//		sleep(3);

		joinDisplay();

		tearDownDisplay();
	} else {
		Activity *rtModelDisplay = createActivity(getRtModelDisplayDescriptor(), messageQueue_blocking);

		// Establish the signal handler
		signal(SIGINT, handleCtrlC);

		// Wait for SIGINT signal
		pause();

		destroyActivity(rtModelDisplay);
	}

main_out:
	return result;
}
