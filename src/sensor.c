/**
 * Sensor helper functions
 *
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

int readBlockable(char *sensorFifo) {
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

int readNonBlockable(char *sensorFile) {
	char input[2];
	struct flock lock;
	memset(input, 0, 2);
	memset(&lock, 0, sizeof(struct flock));
	int fd = open(sensorFile, O_RDONLY);
	lock.l_type = F_RDLCK;
	lock.l_whence = SEEK_SET;
	while(1) {
		if (fcntl(fd, F_SETLKW, &lock) < 0) {
			printf("error\n");
		}
		lseek(fd, 0, SEEK_SET);
		read(fd, input, 1);
		printf("Value: %s\n", input);
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLKW, &lock);
		sleep(3);
	}
	close(fd);
	return 0;
}
