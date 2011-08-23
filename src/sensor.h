/**
 * @brief   Sensor helper functions
 * @file    sensor.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef SENSOR_H_
#define SENSOR_H_

extern int readBlockableSensor(char *sensorFifo);
extern int readNonBlockableSensor(char *sensorFile);

#endif /* SENSOR_H_ */
