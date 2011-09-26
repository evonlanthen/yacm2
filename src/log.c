/**
 * Log messages to syslog
 *
 * @file    syslog.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 22, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <syslog.h>
#include "log.h"

int setUpSyslog(void) {
	openlog("yacm2", LOG_PID|LOG_CONS, LOG_USER);
	return 0;
}

void logInfo(const char *str, ...) {
	va_list ap;
	/* print to sylog: */
	va_start(ap, str);
	vsyslog(LOG_INFO, str, ap);
	va_end(ap);
#ifdef DEBUG
	/* print to stdout as well: */
	printf("LogInfo: ");
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	printf("\n");
#endif
}

void logWarn(const char *str, ...) {
	va_list ap;
	/* print to sylog: */
	va_start(ap, str);
	vsyslog(LOG_WARNING, str, ap);
	va_end(ap);
#ifdef DEBUG
	/* print to stdout as well: */
	printf("LogWarn: ");
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	printf("\n");
#endif
}

void logErr(const char *str, ...) {
	va_list ap;
	/* print to sylog: */
	va_start(ap, str);
	vsyslog(LOG_ERR, str, ap);
	va_end(ap);
#ifdef DEBUG
	/* print to stdout as well: */
	printf("LogErr: ");
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	printf("\n");
#endif
}

void tearDownSyslog(void) {
	closelog();
}
