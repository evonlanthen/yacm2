/**
 * @brief   Submodule RT-model display interface
 * @file    rtModelDisplayRemote.h
 * @version 1.0
 * @authors	Toni Baumann (bauma12@bfh.ch), Ronny Stauffer (staur3@bfh.ch), Elmar Vonlanthen (vonle1@bfh.ch)
 * @date    Sep 19, 2011
 */

#include <rtModelDisplay.h>
#include <rtModelDisplayMessageMapping.h>

// Activity descriptor
static ActivityDescriptor rtModelDisplay = {
	.name = "rtModelDisplay",
	.scope = activityScope_external
};

ActivityDescriptor getRtModelDisplayDescriptor() {
	return rtModelDisplay;
}
