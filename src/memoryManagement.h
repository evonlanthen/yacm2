/**
 * @brief	Memory management
 * @file    memoryManagement.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 30, 2011
 */

#ifndef MEMORYMANAGEMENT_H_
#define MEMORYMANAGEMENT_H_

#include <stdlib.h>

extern void * newObject(void *initializer, size_t size);
extern void deleteObject(void *object);

#endif /* MEMORYMANAGEMENT_H_ */
