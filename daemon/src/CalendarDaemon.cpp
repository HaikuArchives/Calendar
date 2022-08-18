/*
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CalendarDaemon.h"

#include <csignal>
#include <iostream>
#include <time.h>

#include <Alert.h>
#include <DateTime.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <NodeMonitor.h>
#include <Notification.h>
#include <Path.h>
#include <VolumeRoster.h>

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
	app.Run();

	return 0;
}


/*! 
	ReminderEvent Class Definitions
*/


/*ReminderEvent::ReminderEvent(const char* name, const char* place,
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
}*/



/*! 
	CalendarDaemon Class Definitions
*/


CalendarDaemon::CalendarDaemon()
	:
	BApplication(kApplicationSignature),
	fQuitting(false),
	fEventList()
{
	std::cout << "Creating Daemon" << std::endl;

	fEventLock = create_sem(1, "EventLock");
	fNotify = create_sem(0, "Notify");

	BVolumeRoster volRoster;
	volRoster.GetBootVolume(&fQueryVolume);

	BPath homePath;
	find_directory(B_USER_DIRECTORY, &homePath);
	fEventDir = new BDirectory(homePath.Path());

	BPath trashPath;
	find_directory(B_TRASH_DIRECTORY, &trashPath);
	fTrashDir = new BDirectory(trashPath.Path());

	fDBManager = new QueryDBManager();

	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetTitle("Calendar Daemon is Up & Running!");
	notification.SetContent("Secretly Monitoring your Events!");
	notification.Send();

	node_ref nodeRef;
	fEventDir->GetNodeRef(&nodeRef);
	watch_node(&nodeRef, B_WATCH_DIRECTORY, be_app_messenger);

	fEventLoop = spawn_thread(EventLoop, "EventLoop", B_NORMAL_PRIORITY, this);
	resume_thread(fEventLoop);

	SetPulseRate(bigtime_t(1000000));
	//SetFlags(B_PULSE_NEEDED);
}


void
CalendarDaemon::Pulse()
{
	std::cout << "\nPulse!!\n";
}


void
CalendarDaemon::ReadyToRun()
{
	LockEvents();

	fQuery.SetVolume(&fQueryVolume);
	fQuery.PushAttr(START_ATTR);
	fQuery.PushUInt32(time(NULL));
	fQuery.PushOp(B_GE);

	if (fQuery.SetTarget(this) != B_OK)
		std::cout << "Query Target not set" << std::endl;

	fQuery.Fetch();
	entry_ref ref;

	while (fQuery.GetNextRef(&ref) == B_OK)
		AddEventToList(&ref);
	ShowEvents();

	UnlockEvents();
}


CalendarDaemon::~CalendarDaemon()
{
	std::cout << "Stopping Daemon, Clearing Memory, Good Bye! ;)" << std::endl;
	fEventList.MakeEmpty();
	stop_watching(be_app_messenger);
	fQuitting = true;
	Notify();
	
	int32 res;
	wait_for_thread(fEventLoop, &res);
}


void
CalendarDaemon::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case B_QUERY_UPDATE:
		{
			std::cout << "\nEvents Changed - Live Query" << std::endl;
			RefreshEventList();
			break;
		}
		case B_NODE_MONITOR:
		{
			std::cout << "\nAttribute Changed - Node Monitor" << std::endl;
			int32 opCode;
			message->FindInt32("opcode", &opCode);

			if(opCode == B_ATTR_CHANGED)
			{
				RefreshEventList();
				snooze(2000);
			}
			break;
		}
		case B_PULSE:
			Pulse();
			break;
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
	Event* event = _FileToEvent(ref);

	BEntry evEntry(ref);
	bool inTrash = fTrashDir->Contains(&evEntry);
	if (!inTrash) {
		fEventList.AddItem(event);

		BNode node(ref);
		node_ref nodeRef;
		node.GetNodeRef(&nodeRef);
		watch_node(&nodeRef, B_WATCH_ATTR, be_app_messenger);
	}
}


int32
CalendarDaemon::EventLoop(void* data)
{
	CalendarDaemon *app = (CalendarDaemon*)data;

	while (!app->fQuitting) {
		app->LockEvents();

		std::cout << "Thread Running" << std::endl;
		app->fEventList.SortItems(_CompareFunction);

		while (app->fEventList.CountItems() > 0) {
			Event* event = (Event*)app->fEventList.ItemAt(0);

			if (event->GetStartDateTime() > real_time_clock()) {
				std::cout << "Not yet" << std::endl;
				break;
			}

			BString buff = event->GetName();
			buff << "\n\n";
			buff << event->GetPlace();
			BAlert* alert = new BAlert("Hello World!", buff.String(),
							"Okay", NULL, NULL, B_WIDTH_AS_USUAL,
							B_OFFSET_SPACING, B_WARNING_ALERT);
			alert->SetShortcut(0, B_ESCAPE);
			alert->Go();
			app->fEventList.RemoveItemAt((int32)0);
		}

		bigtime_t timeout = -1;
		if (app->fEventList.CountItems() > 0) {
			Event* event = (Event*)app->fEventList.ItemAt(0);
			timeout = (event->GetStartDateTime() - real_time_clock()) * 1000000;
		}
		app->UnlockEvents();

		if (timeout >= 0)
			acquire_sem_etc(app->fNotify, 1, B_RELATIVE_TIMEOUT, timeout);
		else
			acquire_sem(app->fNotify);
	}

	std::cout << "Thread Stopped!\n";
	return 0;
}


void
CalendarDaemon::RefreshEventList()
{
	stop_watching(be_app_messenger);
	LockEvents();

	fQuery.Clear();
	fQuery.SetVolume(&fQueryVolume);
	fQuery.PushAttr(START_ATTR);
	fQuery.PushUInt32(time(NULL));
	fQuery.PushOp(B_GE);

	if (fQuery.SetTarget(this) != B_OK)
		std::cout << "Query Target not set" << std::endl;

	fQuery.Fetch();
	entry_ref ref;
	fEventList.MakeEmpty();

	while(fQuery.GetNextRef(&ref) == B_OK)
		AddEventToList(&ref);
	fEventList.SortItems(_CompareFunction);

	std::cout << "Unlocking after refreshing\n";
	ShowEvents();
	UnlockEvents();

	if (fEventLoop)
		Notify();
}


void
CalendarDaemon::ShowEvents()
{
	if (fEventList.IsEmpty()) {
		std::cout << "The List is empty!" << std::endl;
		return;
	}

	Event* event;
	std::cout << std::endl;
	for (int32 i=0 ; i<fEventList.CountItems() ; ++i) {
		event = fEventList.ItemAt(i);
		std::cout << "Event Name: " << event->GetName() << "\n";
		std::cout << "Event Place: " << event->GetPlace() << "\n\n";
	}
	delete(event);
}


bool
CalendarDaemon::QuitRequested()
{
	return true;
}


Event*
CalendarDaemon::_FileToEvent(entry_ref* ref)
{
	BNode node(ref);
	BEntry entry(ref);
	if (node.InitCheck() != B_OK || entry.InitCheck() != B_OK)
		return NULL;

	BString name = BString();
	BString catName = BString();
	BString idStr = BString();
	BString desc = BString();
	BString place = BString();
	BString statusStr = BString();
	node.ReadAttrString(NAME_ATTR, &name);
	node.ReadAttrString(CATNAME_ATTR, &catName);
	node.ReadAttrString("Calendar:ID", &idStr);
	node.ReadAttrString(DESCRIPTION_ATTR, &desc);
	node.ReadAttrString(PLACE_ATTR, &place);
	node.ReadAttrString("Event:Status", &statusStr);

	time_t start = time(NULL);
	time_t end = time(NULL);
	time_t updated = time(NULL);
	node.ReadAttr(START_ATTR, B_TIME_TYPE, 0, &start, sizeof(time_t));
	node.ReadAttr("Event:End", B_TIME_TYPE, 0, &end, sizeof(time_t));
	node.ReadAttr("Event:Updated", B_TIME_TYPE, 0, &updated, sizeof(time_t));

	return new Event(name.String(), place.String(), desc.String(), false,
		start, end, fDBManager->EnsureCategory(catName.String()), updated, 0,
		idStr.String());
}


int
CalendarDaemon::_CompareFunction(const Event* a, const Event* b)
{
	if (difftime(a->GetStartDateTime(), b->GetStartDateTime()) < 0)
		return -1;
	else if (difftime(a->GetStartDateTime(), b->GetStartDateTime()) > 0)
		return 1;
	else
		return 0;
}
