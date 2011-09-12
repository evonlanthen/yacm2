/**
 * Create timer objects
 *
 * @file    timer.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    May 26, 2011
 */

#ifndef TIMER_H_
#define TIMER_H_

/**
 * Void pointer as handle to the TimerDescriptor structure.
 */
typedef void* TIMER;

/**
 * Set up timer
 *
 * Starts the timer and returns timer description structure
 *
 * @param time Time in milliseconds
 * @return Returns pointer to timer description structure
 */
extern TIMER setUpTimer(unsigned int time);

/**
 * Free timer descriptor structure
 *
 * @param timer Pointer to timer descriptor structure
 * @return Returns TRUE if freeing was successful
 */
extern void abortTimer(TIMER timer);

/**
 * Check if timer is elapsed.
 *
 * @param timer Pointer Timer descriptor structure
 * @return Returns TRUE if time is elapsed and FALSE if not
 */
extern int isTimerElapsed(TIMER timer);

#endif /* TIMER_H_ */
