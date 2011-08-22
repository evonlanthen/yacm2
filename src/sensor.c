/**
 * @brief   Sensor helper functions
 * @file    sensor.c
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
#include "sensor.h"

int readBlockableSensor(char *sensorFifo) {
	char input[2];
	int ret, fd;
	struct flock lock;
	struct pollfd listen_for[1];

	memset(input, 0, 2);
	memset(&lock, 0, sizeof(struct flock));
	fd = open(sensorFifo, O_RDONLY);
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

int readNonBlockableSensor(char *sensorFile, char *buf) {
	char input[80];
	int fd;
	struct flock lock;
	ssize_t bytesRead = 0;
	memset(input, 0, 80);
	memset(&lock, 0, sizeof(struct flock));
	fd = open(sensorFile, O_RDONLY);
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
	logInfo("Value of sensor %s: %s\n", sensorFile, input);
	strcpy(buf, input);
	close(fd);
	return 0;
}
