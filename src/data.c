/**
 * @brief	Data structures
 * @file    data.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#include <string.h>
#include <pthread.h>
#include "data.h"

static struct OperationParameters {
	char *key;
	int value;
} operationParameters[] = {
	{ "milkMaxLacticAcid", 3 },			// 0; 0 - 14 [ph]
	{ "coffeeMotorWarmUpPower", 50 },	// 1; [%]
	{ "coffeeMotorWarmUpTime", 100 },	// 2; [ms]
};
static int operationParametersCount = 3;
static pthread_mutex_t operationParametersLock = PTHREAD_MUTEX_INITIALIZER;

static struct MainParameters {
	char *key;
	int value;
} mainParameters[] = {
	{ "cupFillLevel", 200 },			// 0; [ml]
	{ "coffeePowderAmountPerCup", 30 },	// 1; [mg]
	{ "milkAmountPerCup", 10 },			// 2; [%]
	{ "waterBrewTemperatur", 90 },		// 3; [°C]
	{ "milkCoolingTemperatur", 5 },		// 4; [°C]
};
static int mainParametersCount = 5;
static pthread_mutex_t mainParametersLock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
	int numberOfProducts;
	MachineState machineState;
	// TODO
} OperationData;

typedef struct {
	unsigned long timestamp;
	int event;
} StatisticEntry;

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
	// Critical section
	pthread_mutex_lock(&operationParametersLock);
	for (int i = 0; i < operationParametersCount; i++) {
		if (strcmp(operationParameters[i].key, name) == 0) {
			operationParameters[i].value = value;
			break;
		}
	}
	pthread_mutex_unlock(&operationParametersLock);
}

int getOperationParameter(char *name) {
	int value = -1;
	// Critical section
	pthread_mutex_lock(&operationParametersLock);
	for (int i = 0; i < operationParametersCount; i++) {
		if (strcmp(operationParameters[i].key, name) == 0) {
			value = operationParameters[i].value;
			break;
		}
	}
	pthread_mutex_unlock(&operationParametersLock);
	return value;
}

void setMainParameter(char *name, int value) {
	// Critical section
	pthread_mutex_lock(&mainParametersLock);
	for (int i = 0; i < mainParametersCount; i++) {
		if (strcmp(mainParameters[i].key, name) == 0) {
			mainParameters[i].value = value;
			break;
		}
	}
	pthread_mutex_unlock(&mainParametersLock);
}

int getMainParameter(char *name) {
	int value = -1;
	// Critical section
	pthread_mutex_lock(&mainParametersLock);
	for (int i = 0; i < operationParametersCount; i++) {
		if (strcmp(mainParameters[i].key, name) == 0) {
			value = mainParameters[i].value;
			break;
		}
	}
	pthread_mutex_unlock(&mainParametersLock);
	return 0;
}

int getNumberOfProducts() {
	// Critical section
	pthread_mutex_lock(&operationDataLock);
	int numberOfProducts = operationData.numberOfProducts;
	pthread_mutex_unlock(&operationDataLock);
	return numberOfProducts;
}

void setMachineState(MachineState state) {
	// Critical section
	pthread_mutex_lock(&operationDataLock);
	operationData.machineState = state;
	pthread_mutex_unlock(&operationDataLock);
}

MachineState getMachineState() {
	// Critical section
	pthread_mutex_lock(&operationDataLock);
	int state = operationData.machineState;
	pthread_mutex_unlock(&operationDataLock);
	return state;
}

void addStatisticEntry(int event) {
	// TODO
	// Critical section
	pthread_mutex_lock(&statisticsLock);
	pthread_mutex_unlock(&statisticsLock);
}

