/**
 * @brief   Device helper functions
 * @file    device.c
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 17, 2011
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include "defines.h"
#include "syslog.h"
#include "device.h"

int readBlockableDevice(char *deviceFifo) {
	char input[2];
	int ret, fd;
	struct flock lock;
	struct pollfd listen_for[1];

	memset(input, 0, 2);
	memset(&lock, 0, sizeof(struct flock));
	fd = open(deviceFifo, O_RDONLY);
	listen_for[0].fd = fd;
	listen_for[0].events = POLLIN;
	listen_for[0].revents = 0;

	lock.l_type = F_RDLCK;
	lock.l_whence = SEEK_SET;
	while(1) {
		ret = poll(listen_for, 1, -1);
		//if (fcntl(fd, F_SETLKW, &lock) < 0) {
		//	perror("fcntl");
		//}
		if (ret < 0) {
			perror("poll");
		} else {
			if (listen_for[0].revents == POLLIN) {
				ret = read(fd, input, 1);
				if (ret < 0) perror("read");
				printf("Value: %s\n", input);
				//strcpy(buf, input);
		//lock.l_type = F_UNLCK;
		//fcntl(fd, F_SETLKW, &lock);
			}
		}
		close(fd);
		fd = open("test.fifo", O_RDONLY);
		printf("looping\n");
		//sleep(3);
	}
	close(fd);
	return 0;
}

int readNonBlockableDevice(char *deviceFile) {
	char input[80];
	int fd, i;
	struct flock lock;
	ssize_t bytesRead = 0;
	memset(input, 0, 80);
	memset(&lock, 0, sizeof(struct flock));
	fd = open(deviceFile, O_RDONLY);
	lock.l_type = F_RDLCK;
	lock.l_whence = SEEK_SET;

	if (fcntl(fd, F_SETLKW, &lock) < 0) {
		logErr("fcntl: %s", strerror(errno));
	}
	//lseek(fd, 0, SEEK_SET);
	bytesRead = read(fd, input, 80);
	if (bytesRead < 0) {
		logErr("read: %s", strerror(errno));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &lock);
	close(fd);

	// replace newline with EOF:
	for (i = 0; i < 80; i++) {
		if (input[i] == '\n') {
			input[i] = '\0';
		}
	}
	return atoi(input);
}

int writeNonBlockableDevice(char *deviceFile, char *str, WriteMode mode, int newLine) {
	int fd;
	struct flock lock;
	mode_t openMode = O_WRONLY;
	int bytesWritten = 0;
	memset(&lock, 0, sizeof(struct flock));

	if (mode == wrm_append) {
		openMode |= O_APPEND;
	}
	if ((fd = open(deviceFile, openMode)) < 0) {
		logErr("open: %s", strerror(errno));
		return bytesWritten;
	}
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;

	if (fcntl(fd, F_SETLKW, &lock) < 0) {
		logErr("fcntl: %s", strerror(errno));
		return bytesWritten;
	}
	bytesWritten = write(fd, str, strlen(str));
	if (bytesWritten < 0) {
		logErr("write: %s", strerror(errno));
	}
	if (newLine == TRUE) {
		bytesWritten += write(fd, "\n", 1);
		if (bytesWritten < 0) {
			logErr("write: %s", strerror(errno));
		}
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &lock);
	close(fd);

	return bytesWritten;
}
