/**
 * <short description>
 *
 * @brief   Helper functions to create activities
 * @file    activity.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Aug 15, 2011
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_

#include <sys/stat.h>
#include <pthread.h>
#include <mqueue.h>

#define MAX_ACTIVITY_NAME_LENGTH 32

#define MAX_MESSAGE_LENGTH 400

typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned int DWord;

// Old
typedef struct {
	int intValue;
	char stringValue[128];
} SimpleMessage;

#define COMMON_MESSAGE_CONTENT_DEFINITION_BEGIN MESSAGE_CONTENT_DEFINITION_BEGIN

#define COMMON_MESSAGE_CONTENT_DEFINITION_END(name) MESSAGE_CONTENT_DEFINITION_END(Common, name)

#define MESSAGE_CONTENT_DEFINITION_BEGIN \
	typedef struct {

#define MESSAGE_CONTENT_DEFINITION_END(subsystem, name) \
	} subsystem##name##Content; \
	extern const Byte subsystem##name##Type;

#define COMMON_MESSAGE_CONTENT_REDEFINITION(subsystem, name) \
	typedef Common##name##Content subsystem##name##Content; \
	extern const Byte subsystem##name##Type;

#define MESSAGE_CONTENT_TYPE_MAPPING(subsystem, name, typeId) \
	const Byte subsystem##name##Type = typeId;

#define MESSAGE_DEFINITION_BEGIN \
	typedef struct { \
		Byte type; \
		union {

#define MESSAGE_CONTENT(subsystem, name) \
	subsystem##name##Content subsystem##name;

#define MESSAGE_DEFINITION_END(name) \
		} content; \
	} name##Message;

// Old
#define receiveSimpleMessage_BEGIN(activity) \
	{ \
	ActivityDescriptor senderDescriptor; \
	SimpleMessage message; \
	int result = receiveMessage2(activity, &senderDescriptor, &message, sizeof(message));

#define receiveGenericMessage_BEGIN(activity) \
	{ \
		ActivityDescriptor senderDescriptor; \
		Byte message[MAX_MESSAGE_LENGTH]; \
		int result = receiveMessage2(activity, &senderDescriptor, &message, MAX_MESSAGE_LENGTH); \
		int __attribute__((__unused__)) error = result < 0 ? -result : 0;

#define receiveGenericMessage_END receiveMessage_END

#define receiveMessage_BEGIN(activity, receiver) \
	{ \
		ActivityDescriptor senderDescriptor; \
		receiver##Message message; \
		int result = receiveMessage2(activity, &senderDescriptor, &message, sizeof(message)); \
		int __attribute__((__unused__)) error = result < 0 ? -result : 0;

#define receiveMessage_END \
	}

#define waitForEvent_BEGIN(activity, receiver, timeout) \
	{ \
		ActivityDescriptor senderDescriptor; \
		receiver##Message message; \
		int result = waitForEvent2(activity, &senderDescriptor, &message, sizeof(message), timeout); \
		int __attribute__((__unused__)) error = result < 0 ? -result : 0;

#define waitForEvent_END receiveMessage_END

// Old
#define sendSimpleMessage(sender, receiver, _intValue) \
	sendMessage2(sender, receiver, &(SimpleMessage) { \
		.intValue = _intValue \
	});

#define sendRequest_BEGIN(sender, receiver, _content) \
	sendMessage2(sender, get##receiver##Descriptor(), sizeof(receiver##Message), &(receiver##Message) { \
		.type = receiver##_content##Type, \
		.content.receiver##_content = {

#define sendRequest_END sendMessage_END

#define sendNotification_BEGIN(sender, notifier, receiver, _content) \
	sendMessage2(sender, receiver, sizeof(notifier##Message), &(notifier##Message) { \
		.type = notifier##_content##Type, \
		.content.notifier##_content = {

#define sendNotification_END sendMessage_END

#define sendResponse_BEGIN(sender, responder, _content) \
	sendMessage2(sender, senderDescriptor, sizeof(responder##Message), &(responder##Message) { \
		.type = responder##_content##Type, \
		.content.responder##_content = {

#define sendResponse_END sendMessage_END

#define sendMessage_END \
		} \
	}, messagePriority_medium);

#define MESSAGE_SELECTOR_BEGIN \
	if (0) {

// Obsolete
#define MESSAGE_SELECTOR(message, activityName) \
	} else if (strcmp(message.activity.name, #activityName) == 0) {

#define MESSAGE_BY_SENDER_SELECTOR(senderDescriptor, message, sender) \
	} else if (strcmp(senderDescriptor.name, get##sender##Descriptor().name) == 0) { \
		sender##Message *specificMessage = (sender##Message *)&message;

#define MESSAGE_BY_TYPE_SELECTOR(message, subsystem, _content) \
	} else if ((message).type == subsystem##_content##Type) { \
		subsystem##_content##Content __attribute__((__unused__)) content = (message).content.subsystem##_content;

#define MESSAGE_SELECTOR_ANY \
	} else {

#define MESSAGE_SELECTOR_END \
	}

typedef void (*ActivityRun)(void *activity);

typedef struct {
	unsigned int id;
	char name[MAX_ACTIVITY_NAME_LENGTH];
	ActivityRun setUp;
	ActivityRun run;
	ActivityRun tearDown;
} ActivityDescriptor;

typedef enum {
	messageQueue_blocking = 0,
	messageQueue_nonBlocking
} MessageQueueMode;

typedef struct {
	ActivityDescriptor *descriptor;
	pthread_t thread;
	mqd_t messageQueue;
	MessageQueueMode messageQueueMode;
	int polling;
} Activity;

typedef enum {
	messagePriority_low = 0,
	messagePriority_medium,
	messagePriority_high,
} MessagePriority;

// Activity creation/destruction API
Activity *createActivity(ActivityDescriptor descriptor, MessageQueueMode messageQueueMode);
void destroyActivity(Activity *activity);

// Old messaging API
int waitForEvent(Activity *activity, char *buffer, unsigned long length, unsigned int timeout);
int receiveMessage(void *_receiver, char *buffer, unsigned long length);
int sendMessage(ActivityDescriptor activity, char *buffer, unsigned long length, MessagePriority priority);

// New messaging API
int waitForEvent2(Activity *activity, ActivityDescriptor *senderDescriptor, void *buffer, unsigned long length, unsigned int timeout);
//int receiveMessage2(void *_receiver, char *senderName, char *buffer, unsigned long length);
int receiveMessage2(void *_receiver, ActivityDescriptor *senderDescriptor, void *buffer, unsigned long length);
int sendMessage2(void *_sender, ActivityDescriptor activity, unsigned long length, void *buffer, MessagePriority priority);

COMMON_MESSAGE_CONTENT_DEFINITION_BEGIN
COMMON_MESSAGE_CONTENT_DEFINITION_END(InitCommand)

COMMON_MESSAGE_CONTENT_DEFINITION_BEGIN
COMMON_MESSAGE_CONTENT_DEFINITION_END(OffCommand)

COMMON_MESSAGE_CONTENT_DEFINITION_BEGIN
COMMON_MESSAGE_CONTENT_DEFINITION_END(AbortCommand)

COMMON_MESSAGE_CONTENT_DEFINITION_BEGIN
	unsigned int code;
	unsigned int errorCode;
COMMON_MESSAGE_CONTENT_DEFINITION_END(Result)

#endif /* ACTIVITY_H_ */
