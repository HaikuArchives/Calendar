/*
 * Copyright 2022, Harshit Sharma <harshits908@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef CALENDAR_DAEMON_H
#define CALENDAR_DAEMON_H

#include <Application.h>
#include <OS.h>
#include <Volume.h>

#include "QueryDBManager.h"

/*!
	CalendarDaemon Class Declaration
*/


class CalendarDaemon : public BApplication
{
public:
		CalendarDaemon();
		~CalendarDaemon();

	void			MessageReceived(BMessage* message);
	bool			QuitRequested();

	void			WatchEvent(entry_ref* ref);
	void			UnwatchEvent(entry_ref* ref);
	void			ShowEvents();
	void			AddEventToList(entry_ref* ref);
	void			RemoveEventFromList(entry_ref* ref);

private:

	static int _CompareFunction(const Event* a, const Event* b);

	BString				fEventDir;
	BVolume				fQueryVolume;
	EventList*			fEventList;
	QueryDBManager*		fDBManager;
	
};

#endif
