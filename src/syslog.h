/**
 * <short description>
 *
 * <long description>
 *
 * @file    syslog.h
 * @version 0.1
 * @author  Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef SYSLOG_H_
#define SYSLOG_H_

/**
 * Log messages to syslog
 *
 * @file    syslog.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include "syslog.h"

extern int setUpSyslog(void);
extern void logInfo(char *str);
extern void logWarn(char *str);
extern void logErr(char *str);
extern void tearDownSyslog(void);

#endif /* SYSLOG_H_ */
