/**
 * @brief	Data structures
 * @file    data.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef DATA_H_
#define DATA_H_

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
