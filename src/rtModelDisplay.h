/**
 * @brief   Submodule RT-model display
 * @file    rtModelDisplay.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 19, 2011
 */

#ifndef RTMODELDISPLAY_H_
#define RTMODELDISPLAY_H_

#include <activity.h>

MESSAGE_CONTENT_DEFINITION_BEGIN
	char message[128];
MESSAGE_CONTENT_DEFINITION_END(RtModelDisplay, ShowMessageCommand)

MESSAGE_DEFINITION_BEGIN
	MESSAGE_CONTENT(RtModelDisplay, ShowMessageCommand)
MESSAGE_DEFINITION_END(RtModelDisplay)

ActivityDescriptor getRtModelDisplayDescriptor();

#endif /* RTMODELDISPLAY_H_ */
