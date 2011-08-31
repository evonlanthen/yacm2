/**
 * @brief	Memory management
 * @file    memoryManagement.c
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 30, 2011
 */

#include <string.h>
#include "memoryManagement.h"

/**
 * Instanciates and initializes a new object.
 */
void * newObject(void *initializer, size_t size) {
	void *object = malloc(size);
	memcpy(object, initializer, size);

	return object;
}

/**
 * Deletes an object
 */
void deleteObject(void *object) {
	free(object);
}
