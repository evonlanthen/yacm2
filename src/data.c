/**
 * @brief	Data structures
 * @file    data.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include "data.h"
#include <pthread.h>

struct OperationParameters {
	char *key;
	int value;
} operationParameters[] = {
		{ "milkMaxLacticAcid", 3 },			// 0 - 14 ph
		{ "coffeeMotorWarmUpPower", 50 },	// %
		{ "coffeeMotorWarmUpTime", 100 },	// ms
};

typedef struct {
	int cupFillLevel; // ml
	int coffeePowderAmountPerCup; // mg
	int milkAmountPerCup; // %
	int waterBrewTemperatur; // °C
	int milkCoolingTemperatur; // °C
} MainParameters;

typedef struct {
	int numberOfProducts;
	MachineState machineState;
	// TODO
} OperationData;

typedef struct {
	unsigned long timestamp;
	int event;
} StatisticEntry;

/*
static OperationParameters operationParameters = {
	.milkMaxLacticAcid = 3,
	.coffeeMotorWarmUpPower = 50,
	.coffeeMotorWarmUpTime = 100
};
*/

static pthread_mutex_t operationParametersLock = PTHREAD_MUTEX_INITIALIZER;

static MainParameters mainParameters = {
	.cupFillLevel = 200,
	.coffeePowderAmountPerCup = 30,
	.milkAmountPerCup = 10,
	.waterBrewTemperatur = 90,
	.milkCoolingTemperatur = 5
};
static pthread_mutex_t mainParametersLock = PTHREAD_MUTEX_INITIALIZER;

static OperationData operationData = {
	.numberOfProducts = 3,
	.machineState = 0
};
static pthread_mutex_t operationDataLock = PTHREAD_MUTEX_INITIALIZER;

static StatisticEntry statistic[1000];
static pthread_mutex_t statisticsLock = PTHREAD_MUTEX_INITIALIZER;

void setUpData() {
	// TODO Read parameters from file
	//pthread_mutex_lock();
	//pthread_mutex_unlock();
}

void tearDownData() {
	// TODO Save parameters to file
	//pthread_mutex_lock();
	//pthread_mutex_unlock();
}

void setOperationParameter(char *name, int value) {
	// TODO
	// Critical section
	pthread_mutex_lock(&operationParametersLock);
	pthread_mutex_unlock(&operationParametersLock);
}

int getOperationParameter(char *name) {
	// TODO
	// Critical section
	pthread_mutex_lock(&operationParametersLock);
	pthread_mutex_unlock(&operationParametersLock);
	return 0;
}

void setMainParameter(char *name, int value) {
	// TODO
	// Critical section
	pthread_mutex_lock(&mainParametersLock);
	pthread_mutex_unlock(&mainParametersLock);
}

int getMainParameter(char *name) {
	// TODO
	// Critical section
	pthread_mutex_lock(&mainParametersLock);
	pthread_mutex_unlock(&mainParametersLock);
	return 0;
}

int getNumberOfProducts() {
	// TODO
	// Critical section
	pthread_mutex_lock(&operationDataLock);

	pthread_mutex_unlock(&operationDataLock);
	return operationData.numberOfProducts;
}

void setMachineState(MachineState state) {
	// TODO
	// Critical section
	pthread_mutex_lock(&operationDataLock);

	pthread_mutex_unlock(&operationDataLock);
	return operationData.machineState;
}

MachineState getMachineState() {
	// TODO
	// Critical section
	pthread_mutex_lock(&operationDataLock);
	pthread_mutex_unlock(&operationDataLock);
	return operationData.machineState;
}

void addStatisticEntry(int event) {
	// TODO
	// Critical section
	pthread_mutex_lock(&statisticsLock);
	pthread_mutex_unlock(&statisticsLock);
}

