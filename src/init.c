/**
 * <short description>
 *
 * <long description>
 *
 * @file    init.c
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <unistd.h>
#include "syslog.h"
#include "activity.h"
#include "mainController.h"
#include "coffeeSupply.h"

int main(int argc, char **argv) {
	setUpSyslog();
	printf("Set up subsystems...\n");
	logInfo("Set up subsystems...");
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
