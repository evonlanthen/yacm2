/**
 * <short description>
 *
 * <long description>
 *
 * @file    data.c
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

typedef struct {
	int milkMaxLacticAcid; // 0 - 14 ph
	int coffeeMotorWarmUpPower; // %
	int coffeeMotorWarmUpTime; // ms
} OperationParameters;

typedef struct {
	int cupFillLevel; // ml
	int coffeePowderAmountPerCup; // mg
	int milkAmountPerCup; // %
	int waterBrewTemperatur; // °C
	int milkCoolingTemperatur; // °C
} MainParameters;

typedef struct {
	int numberOfProducts;
	int machineState;
	// TODO
} OperationData;

typedef struct {
	unsigned long timestamp;
	int event;
} StatisticEntry;

static StatisticEntry statistic[1000];
static OperationParameters operationParameters = {
	.milkMaxLacticAcid = 3,
	.coffeeMotorWarmUpPower = 50,
	.coffeeMotorWarmUpTime = 100
};
static MainParameters mainParameters = {
	.cupFillLevel = 200,
	.coffeePowderAmountPerCup = 30,
	.milkAmountPerCup = 10,
	.waterBrewTemperatur = 90,
	.milkCoolingTemperatur = 5
};
static OperationData operationData = {
	.numberOfProducts = 3,
	.machineState = 0
};

void setUpData() {
	// TODO Read parameters from file
}

void tearDownData() {
	// TODO Save parameters to file
}

void setOperationParameter(char *name, int value) {
	// TODO
	// Critical section
}

int getOperationParameter(char *name) {
	// TODO
	// Critical section
	return 0;
}

void setMainParameter(char *name, int value) {
	// TODO
	// Critical section
}

int getMainParameter(char *name) {
	// TODO
	// Critical section
	return 0;
}

int getNumberOfProducts() {
	// TODO
	// Critical section
	return operationData.numberOfProducts;
}

int getMachineState() {
	// TODO
	// Critical section
	return operationData.machineState;
}

void addStatisticEntry(int event) {
	// TODO
	// Critical section
}

