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
#include "coffeeSupply.h"
#include "waterSupply.h"
#include "milkSupply.h"
#include "userInterface.h"
#include "serviceInterface.h"
#include "mainController.h"

int main(int argc, char **argv) {
	setUpSyslog();
	printf("[init] Setting up subsystems...\n");
	Activity *coffeeSupply = createActivity(getCoffeeSupplyDescriptor());
	Activity *waterSupply = createActivity(getWaterSupplyDescriptor());
	Activity *milkSupply = createActivity(getMilkSupplyDescriptor());
	Activity *userInterface = createActivity(getUserInterfaceDescriptor());
	Activity *serviceInterface = createActivity(getServiceInterfaceDescriptor());
	Activity *mainController = createActivity(getMainControllerDescriptor());

	sleep(60);

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
