/*
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CalendarDaemon.h"

#include <iostream>

#include <Directory.h>
#include <FindDirectory.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <VolumeRoster.h>

#define EVENT_DIRECTORY "config/settings/Calendar/events"

const char* kApplicationSignature = "application/x-vnd.CalendarDaemon";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Daemon"


int main()
{
	CalendarDaemon app;
	app.Run();

	return 0;
}


/*! 
	CalendarDaemon Class Definitions
*/


CalendarDaemon::CalendarDaemon()
	:
	BApplication(kApplicationSignature)
{
	std::cout << "Creating Daemon..." << std::endl;

	BVolumeRoster volRoster;
	volRoster.GetBootVolume(&fQueryVolume);

	BPath homeDir;
	find_directory(B_USER_DIRECTORY, &homeDir);
	fEventDir = homeDir.Path();
	fEventDir << "/" << EVENT_DIRECTORY;

	BDirectory directory(fEventDir.String());

	if (directory.InitCheck() == B_OK) {
		node_ref nodeRef;
		directory.GetNodeRef(&nodeRef);
		watch_node(&nodeRef, B_WATCH_DIRECTORY, be_app_messenger);
	} else
		std::cout << "Events Directory not found!!" << std::endl;
}


CalendarDaemon::~CalendarDaemon()
{
	std::cout << "Stopping Daemon, Good Bye! ;)" << std::endl;
	stop_watching(be_app_messenger);
}


void
CalendarDaemon::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case B_NODE_MONITOR:
		{
			std::cout << "\nNode Monitor Message Received!" << std::endl;
			int32 opCode;

			if (message->FindInt32("opcode", &opCode) == B_OK) {
				switch (opCode) {
					case B_ENTRY_CREATED:
						std::cout << "New Event Created!" << std::endl;
						break;
					case B_ENTRY_REMOVED:
					case B_ENTRY_MOVED:
						std::cout << "An Event Removed!" << std::endl;
						break;
				}
			}
			else
				std::cout << "Op Code not found!" << std::endl;
			break;
		}
		case B_QUIT_REQUESTED:
			QuitRequested();
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}


bool
CalendarDaemon::QuitRequested()
{
	return true;
}
