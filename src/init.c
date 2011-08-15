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
#include "activity.h"
#include "mainController.h"

int main(int argc, char **argv) {
	printf("Set up subsystems...\n");
	Activity *mainController = createActivity(getMainControllerDescriptor());

	sleep(10);

	printf("Tear down subsystems...\n");
	destroyActivity(mainController);
	printf("...done.\n");

	return 0;
}
