/**
 * @brief   Initial system setup
 * @file    init.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "syslog.h"
#include "activity.h"
#include "coffeeSupply.h"
#include "waterSupply.h"
#include "milkSupply.h"
#include "userInterface.h"
#include "serviceInterface.h"
#include "mainController.h"

static void sigCtrlC(int sig)
{
	/* do nothing here */
	;
}

int main(int argc, char **argv) {
	printf("[init] Setting up subsystems...\n");
	setUpSyslog();
	Activity *coffeeSupply = createActivity(getCoffeeSupplyDescriptor(), messageQueue_blocking);
	Activity *waterSupply = createActivity(getWaterSupplyDescriptor(), messageQueue_nonBlocking);
	Activity *milkSupply = createActivity(getMilkSupplyDescriptor(), messageQueue_blocking);
	Activity *userInterface = createActivity(getUserInterfaceDescriptor(), messageQueue_blocking);
	Activity *serviceInterface = createActivity(getServiceInterfaceDescriptor(), messageQueue_blocking);
	Activity *mainController = createActivity(getMainControllerDescriptor(), messageQueue_blocking);

	/* Establish the signal handler. */
	(void) signal(SIGINT, sigCtrlC);
	/* wait for signal SIGINT: */
	pause();

	printf("[init] Tearing down subsystems...\n");
	destroyActivity(mainController);
	destroyActivity(serviceInterface);
	destroyActivity(userInterface);
	destroyActivity(milkSupply);
	destroyActivity(waterSupply);
	destroyActivity(coffeeSupply);
	printf("[init] ...done. (tear down subsystems)\n");
	tearDownSyslog();
	return 0;
}
