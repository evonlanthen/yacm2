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

int setUpSyslog(void) {
	openlog("yacm2", LOG_PID|LOG_CONS, LOG_USER);
	return 0;
}

void logInfo(char *str) {
	syslog(LOG_INFO, "%s", str);
}

void logWarn(char *str) {
	syslog(LOG_WARNING, "%s", str);
}

void logErr(char *str) {
	syslog(LOG_ERR, "%s", str);
}

void tearDownSyslog(void) {
	closelog();
}
