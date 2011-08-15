/**
 * <short description>
 *
 * <long description>
 *
 * @file    activity.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_

typedef pthread_t Activity;
typedef void *(*ActivityRun)();

Activity *createActivity(ActivityRun run);
void destroyActivity(Activity *activity);

#endif /* ACTIVITY_H_ */
