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
	BApplication(kApplicationSignature),
	fEventList()
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

		entry_ref ref;
		while (directory.GetNextRef(&ref) == B_OK) {
			BNode node(&ref);
			node_ref nodeRef;
			node.GetNodeRef(&nodeRef);
			watch_node(&nodeRef, B_WATCH_ATTR, be_app_messenger);
		}
	} else
		std::cout << "Events Directory not found!!" << std::endl;

	fDBManager = new QueryDBManager();

	fEventList = fDBManager->GetEventsToNotify(BDateTime::CurrentDateTime(B_LOCAL_TIME));
	fEventList->SortItems(_CompareFunction);
	ShowEvents();
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
			ino_t node;
			message->FindInt32("opcode", &opCode);
			message->FindInt64("node", &node);

			switch (opCode) {
				case B_ENTRY_CREATED: {
					std::cout << "New Event Created!" << std::endl;
					entry_ref ref;
					const char* name;

					message->FindInt32("device", &ref.device);
					message->FindInt64("directory", &ref.directory);
					message->FindString("name", &name);
					ref.set_name(name);

					AddEventToList(&ref);
				} break;
				//case B_ENTRY_REMOVED:
				case B_ENTRY_MOVED:
					std::cout << "An Event Removed!" << std::endl;
					break;
				case B_ATTR_CHANGED:
					std::cout << "Attribute Changed!" << std::endl;
					break;
			}
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


void
CalendarDaemon::WatchEvent(entry_ref* ref)
{
	BNode node(ref);
	node_ref nodeRef;
	node.GetNodeRef(&nodeRef);
	watch_node(&nodeRef, B_WATCH_ATTR, be_app_messenger);
}


void
CalendarDaemon::UnwatchEvent(entry_ref* ref)
{
	BNode node(ref);
	node_ref nodeRef;
	node.GetNodeRef(&nodeRef);
	watch_node(&nodeRef, B_STOP_WATCHING, be_app_messenger);
}


void
CalendarDaemon::AddEventToList(entry_ref* ref)
{
	Event* event = fDBManager->FileToEvent(ref);
	fEventList->AddItem(event);
	fEventList->SortItems(_CompareFunction);
	WatchEvent(ref);
	ShowEvents();
}


void
CalendarDaemon::RemoveEventFromList(entry_ref* ref)
{
	BNode node(ref);
	node_ref nodeRef;
	node.GetNodeRef(&nodeRef);
	watch_node(&nodeRef, B_STOP_WATCHING, be_app_messenger);
}


void
CalendarDaemon::ShowEvents()
{
	if (fEventList->IsEmpty()) {
		std::cout << "The List is empty!" << std::endl;
		return;
	}

	Event* event;
	std::cout << std::endl;
	for (int32 i=0 ; i<fEventList->CountItems() ; ++i) {
		std::cout << "hue\n";
		event = fEventList->ItemAt(i);
		std::cout << "Event Name: " << event->GetName() << "\n";
		std::cout << "Event Place: " << event->GetPlace() << "\n\n";
	}
}


int
CalendarDaemon::_CompareFunction(const Event* a, const Event* b)
{
	if (difftime(a->GetReminderTime(), b->GetReminderTime()) < 0)
		return -1;
	else if (difftime(a->GetReminderTime(), b->GetReminderTime()) > 0)
		return 1;
	else
		return 0;
}
