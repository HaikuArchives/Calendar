/*
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CalendarDaemon.h"

#include <iostream>

#include <Alert.h>
#include <Catalog.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <StringFormat.h>
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

		entry_ref ref;
		while (directory.GetNextRef(&ref) == B_OK) {
			BNode node(&ref);
			node_ref nodeRef;
			node.GetNodeRef(&nodeRef);
			watch_node(&nodeRef, B_WATCH_ATTR, be_app_messenger);
		}
	} else
		std::cout << "Events Directory not found!!" << std::endl;

	fEventList = fDBManager.GetEventsToNotify(BDateTime::CurrentDateTime(B_LOCAL_TIME));
	fEventList->SortItems(_CompareFunction);
	ShowEvents();
}


void
CalendarDaemon::ReadyToRun()
{
	if (fEventList->CountItems() > 0) {
		Event* event = fEventList->ItemAt(0);
		bigtime_t timeout;		
		timeout = (event->GetReminderTime() - real_time_clock()) * 1000000;
		
		BMessage* notifyMessage = new BMessage(kEventNotify);
		notifyMessage->AddString("name", event->GetName());
		notifyMessage->AddString("place", event->GetPlace());
		notifyMessage->AddInt64("deltaTime", event->GetStartDateTime() - event->GetReminderTime());

		fMessageRunner = new BMessageRunner(be_app_messenger, notifyMessage, timeout, -1);
		if (fMessageRunner->InitCheck() != B_OK)
			std::cout << "MessageRunner Not Initialized!\n";
		std::cout << "\nEvent " << event->GetName() << " going off in " << timeout/100000 << " seconds\n";
	}
}


CalendarDaemon::~CalendarDaemon()
{
	std::cout << "Stopping Daemon, Good Bye! ;)" << std::endl;
	stop_watching(be_app_messenger);
	delete(fEventList);
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
				case B_ENTRY_CREATED:
				case B_ENTRY_REMOVED:
				case B_ENTRY_MOVED: {
					std::cout << "Refreshing Events List!\n";
					RefreshEventList();
					ShowEvents();
				} break;
			}
			break;
		}
		case kEventNotify: {
			const char* name;
			const char* place;
			time_t deltaTime;
			message->FindString("name", &name);
			message->FindString("place", &place);
			message->FindInt64("deltaTime", &deltaTime);

			SendAlert(name, place, deltaTime);

			fEventList->RemoveItemAt((int32)0);
			
			if (fEventList->CountItems() > 0) {
				Event* event = fEventList->ItemAt(0);
				bigtime_t timeout;		
				timeout = (event->GetReminderTime() - real_time_clock()) * 1000000;

				BMessage* notifyMessage = new BMessage(kEventNotify);
				notifyMessage->ReplaceString("name", event->GetName());
				notifyMessage->ReplaceString("place", event->GetPlace());
				notifyMessage->ReplaceInt64("deltaTime", event->GetStartDateTime() - event->GetReminderTime());

				fMessageRunner->SetInterval(timeout);
				std::cout << "\nEvent " << event->GetName() << " going off in " << timeout << " seconds\n";
			}
		} break;
		case B_QUIT_REQUESTED:
			QuitRequested();
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}


void
CalendarDaemon::SendAlert(const char* name, const char* place, time_t deltaTime)
{
	BString alertText(B_TRANSLATE("Calendar notification!\n\n"
		"Event:\t%eventName%\n"
		"Place:\t%eventPlace%\n\n"
		"The event starts in "));

	alertText.ReplaceAll("%eventName%", name);
	alertText.ReplaceAll("%eventPlace%", place);
 
	if (deltaTime%3600 == 0) {
		deltaTime /= 3600;
		static BStringFormat format(B_TRANSLATE("{0, plural,"
		"=1{1 hour!}"
		"other{# hours!}}"));
		format.Format(alertText, deltaTime);
	} else if (deltaTime%60 == 0) {
		deltaTime /= 60;
		static BStringFormat format(B_TRANSLATE("{0, plural,"
		"=1{1 minute!}"
		"other{# minutes!}}"));
		format.Format(alertText, deltaTime);
	} else {
		static BStringFormat format(B_TRANSLATE("{0, plural,"
		"=1{1 second!}"
		"other{# seconds!}}"));
		format.Format(alertText, deltaTime);
	}
	BAlert* alert = new BAlert("Reminder!", alertText.String(),
					"OK", NULL, NULL, B_WIDTH_AS_USUAL,
					B_OFFSET_SPACING, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	alert->Go();
}


bool
CalendarDaemon::QuitRequested()
{
	return true;
}


void
CalendarDaemon::RefreshEventList()
{
	delete(fEventList);
	fEventList = fDBManager.GetEventsToNotify(BDateTime::CurrentDateTime(B_LOCAL_TIME));
	fEventList->SortItems(_CompareFunction);
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
