/**
 * <short description>
 *
 * <long description>
 *
 * @file    data.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef DATA_H_
#define DATA_H_

/**
 * Represents the coffee maker state.
 */
typedef enum {
	machineState_off,
	machineState_initializing,
	machineState_idle,
	machineState_producing
} MachineState;

void setUpData();
void tearDownData();

void setOperationParameter(char *name, int value);
int getOperationParameter(char *name);

void setMainParameter(char *name, int value);
int getMainParameter(char *name);

int getNumberOfProducts();

void setMachineState(MachineState state);
MachineState getMachineState();

void addStatisticEntry(int event);

#endif /* DATA_H_ */
