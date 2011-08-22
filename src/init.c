/**
 * @brief   Initial system setup
 * @file    init.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <unistd.h>
#include "syslog.h"
#include "activity.h"
#include "mainController.h"
#include "coffeeSupply.h"

int main(int argc, char **argv) {
	int i = 42;
	setUpSyslog();
	printf("Set up subsystems...\n");
	logInfo("Set up subsystems (%d)...", i);
	Activity *coffeeSupply = createActivity(getCoffeeSupplyDescriptor());
	Activity *mainController = createActivity(getMainControllerDescriptor());

	sleep(60);

	printf("Tear down subsystems...\n");
	destroyActivity(mainController);
	destroyActivity(coffeeSupply);
	printf("...done. (tear down subsystems)\n");
	tearDownSyslog();
	return 0;
}
