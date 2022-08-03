/*
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CalendarDaemon.h"

#include <FindDirectory.h>
#include <Notification.h>
#include <Path.h>
#include <iostream>
#include <csignal>

#define EVENT_DIRECTORY	"config/settings/Calendar/events"
#define START_ATTR		"Event:Start"
#define DESCRIPTION_ATTR "Event:Description"

const char* kApplicationSignature = "application/x-vnd.CalendarDaemon";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"


void signalHandler(int signum)
{
	std::cout << "Interrupted Signal: " << signum << std::endl;
	exit(signum);
}

int main()
{
	signal(SIGINT, signalHandler);
	
	CalendarDaemon app;
	app.SayHello("Harshit");
	
	while(1)
	{
		std::cout << "Going to Sleep...\n";
		sleep(1);
	}
	
	return(0);
}


CalendarDaemon::CalendarDaemon()
	:	BApplication(kApplicationSignature)
{
	BPath homeDir;
	find_directory(B_USER_DIRECTORY, &homeDir);
	
	fEventDir = homeDir.Path();
	fEventDir << "/" << EVENT_DIRECTORY;
	
	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetTitle("Calendar Daemon");
	notification.SetContent(fEventDir);
	
	notification.Send();
	
	std::cout << "Notification Sent!" << std::endl;
}

CalendarDaemon::~CalendarDaemon()
{
	std::cout << "Bye Bye" << std::endl;
}

void
CalendarDaemon::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
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
	return(true);
}

void CalendarDaemon::SayHello(const char* name)
{
	std::cout << "Hello " << name << std::endl;
}
