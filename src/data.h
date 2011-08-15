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

void setUp();
void tearDown();

void setOperationParameter(char *name, int value);
int getOperationParameter(char *name);

void getMainParameter(char *name, int value);
int getMainParameter(char *name);

int getNumberOfProducts();
int getMachineState();

void addStatisticEntry(int event);


#endif /* DATA_H_ */
