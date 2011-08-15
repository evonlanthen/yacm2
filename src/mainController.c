/**
 * <short description>
 *
 * <long description>
 *
 * @file    mainController.c
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <stdio.h>
#include <unistd.h>
#include "activity.h"

static void setUpMainController(void *);
static void runMainController(void *);
static void tearDownMainController(void *);

static ActivityDescriptor mainController = {
		.name = "mainController",
		.setUp = setUpMainController,
		.run = runMainController,
		.tearDown = tearDownMainController
};

ActivityDescriptor getMainControllerDescriptor() {
	return mainController;
}

static void setUpMainController(void *par) {
	printf("Set up...\n");
}

static void runMainController(void *par) {
	printf("Running...\n");
	sleep(15);
}

static void tearDownMainController(void *par) {
	printf("Tear down...\n");
}
