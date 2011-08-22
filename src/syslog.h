/**
 * @brief   Log messages to syslog
 * @file    syslog.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#ifndef SYSLOG_H_
#define SYSLOG_H_

extern int setUpSyslog(void);
extern void logInfo(const char *format, ...);
extern void logWarn(const char *format, ...);
extern void logErr(const char *format, ...);
extern void tearDownSyslog(void);

#endif /* SYSLOG_H_ */
