/*
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CalendarDaemon.h"

#include <DateTime.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Notification.h>
#include <Path.h>
#include <iostream>
#include <csignal>
#include <time.h>

#define EVENT_DIRECTORY		"config/settings/Calendar/events"
#define START_ATTR			"Event:Start"
#define DESCRIPTION_ATTR 	"Event:Description"
#define PLACE_ATTR			"Event:Place"
#define NAME_ATTR			"Event:Name"
#define CATNAME_ATTR		"Event:Category"

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
	
	return(0);
}


/*! 
	ReminderEvent Class Definitions
*/


ReminderEvent::ReminderEvent(const char* name, const char* place,
	const char* description, const char* catName, time_t start)
{
	fName = BString(name);
	fPlace = BString(place);
	fDescription = BString(description);
	fCatName = BString(catName);
	fStart = start;
}


ReminderEvent::ReminderEvent(ReminderEvent &event)
{
	fName = event.GetName();
	fPlace = event.GetPlace();
	fDescription = event.GetDescription();
	fCatName = event.GetCatName();
	fStart = event.GetStartDateTime();
}


time_t
ReminderEvent::GetStartDateTime() const
{
	return fStart;
}


void
ReminderEvent::SetStartDateTime(time_t start)
{
	fStart = start;
}


const char*
ReminderEvent::GetName() const
{
	return fName.String();
}


void
ReminderEvent::SetName(const char* name)
{
	fName = BString(name);
}


const char*
ReminderEvent::GetPlace() const
{
	return fPlace.String();
}


void
ReminderEvent::SetPlace(const char* place)
{
	fPlace = BString(place);
}


const char*
ReminderEvent::GetDescription() const
{
	return fDescription.String();
}


void
ReminderEvent::SetDescription(const char* description)
{
	fDescription = BString(description);
}


const char*
ReminderEvent::GetCatName() const
{
	return fCatName.String();
}


void
ReminderEvent::SetCatName(const char* catName)
{
	fCatName = BString(catName);
}



/*! 
	CalendarDaemon Class Definitions
*/


CalendarDaemon::CalendarDaemon()
	:	BApplication(kApplicationSignature)
{
	std::cout << "Creating Daemon" << std::endl;
	
	fEventList = new ReminderEventList(20, true);
	
	BPath homeDir;
	find_directory(B_USER_DIRECTORY, &homeDir);
	
	fEventDir = homeDir.Path();
	fEventDir << "/" << EVENT_DIRECTORY;
	
	BDirectory directory(fEventDir.String());
	if(directory.InitCheck() == B_ENTRY_NOT_FOUND)
	{
		std::cerr << "Failed to access the Events Directory" << std::endl;
	}
	else if(directory.InitCheck() == B_OK)
	{
		entry_ref ref;
		while(directory.GetNextRef(&ref) == B_OK)
			AddEventToList(&ref);
	}
	
	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetTitle("Found the Directory ;)");
	notification.SetContent(fEventDir.String());
	
	notification.Send();
	
	std::cout << "Notification Sent!" << std::endl;
}


CalendarDaemon::~CalendarDaemon()
{
	std::cout << "Stopping Daemon, Clearing Memory, Good Bye! ;)" << std::endl;
	delete(fEventList);
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


void
CalendarDaemon::AddEventToList(entry_ref* ref)
{
	ReminderEvent* event = _FileToReminderEvent(ref);
	
	std::cout << "\nEvent Name: " << event->GetName() << std::endl;
	std::cout << "\nEvent Place: " << event->GetPlace() << std::endl;
	
	fEventList->AddItem(event);
}


bool
CalendarDaemon::QuitRequested()
{
	return(true);
}


ReminderEvent*
CalendarDaemon::_FileToReminderEvent(entry_ref* ref)
{
	BNode node(ref);
	BEntry entry(ref);
	if (node.InitCheck() != B_OK || entry.InitCheck() != B_OK)
		return NULL;

	BString name = BString();
	BString catName = BString();
	BString desc = BString();
	BString place = BString();
	node.ReadAttrString(NAME_ATTR, &name);
	node.ReadAttrString(CATNAME_ATTR, &catName);
	node.ReadAttrString(DESCRIPTION_ATTR, &desc);
	node.ReadAttrString(PLACE_ATTR, &place);

	time_t start = time(NULL);
	node.ReadAttr(START_ATTR, B_TIME_TYPE, 0, &start, sizeof(time_t));

	return new ReminderEvent(name.String(), place.String(), desc.String(),
		catName.String(), start);
}


int
CalendarDaemon::_CompareFunction(ReminderEvent* a, ReminderEvent* b)
{
	if(difftime(a->GetStartDateTime(), b->GetStartDateTime()) < 0)
		return(1);
	else if(difftime(a->GetStartDateTime(), b->GetStartDateTime()) > 0)
		return(-1);
	else
		return(0);
}
