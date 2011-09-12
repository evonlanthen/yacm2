/**
 * @brief   Preprocessor definitions
 * @file    defines.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    May 20, 2011
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/**
 * Define the BOARD to use (CARME or ORCHID)
 */
//#define CARME
#ifndef CARME
	#define ORCHID
#endif


/**
 * Define symbol for the boolean value 'true'.
 */
#define TRUE 1
/**
 * Define symbol for the boolean value 'false'.
 */
#define FALSE 0

/**
 * Define NULL pointer
 */
#ifndef NULL
 #define NULL ((void*)0)
#endif

/**
 * Define end of file value
 */
#define EOF (-1)

#define INIT_COMMAND 1
#define OFF_COMMAND 2
#define ABORT_COMMAND 3
#define OK_RESULT 101
#define NOK_RESULT 201


#endif /* DEFINES_H_ */
