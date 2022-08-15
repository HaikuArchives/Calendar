/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef CALENDAR_DAEMON_H
#define CALENDAR_DAEMON_H

#include <iostream>

#include <Application.h>
#include <OS.h>
#include <Query.h>
#include <Volume.h>

#include "QueryDBManager.h"

enum
{
	kEventAdded,
	kEventRemoved,
	kEventChanged,
	kEventNotify,
};

class BMessage;
class BString;
class BVolume;
class BQuery;
class BDirectory;


/*!
	ReminderEvent Class Declaration
*/

/*class ReminderEvent
{
public:
	ReminderEvent(const char* name, const char* place,
		const char* description, const char* catName, time_t start);
	ReminderEvent(ReminderEvent& event);

	time_t		GetStartDateTime() const;
	void		SetStartDateTime(time_t start);

	const char*	GetName() const;
	void		SetName(const char* name);

	const char*	GetPlace() const;
	void		SetPlace(const char* place);

	const char*	GetDescription() const;
	void		SetDescription(const char* description);

	const char*	GetCatName() const;
	void		SetCatName(const char* catName);

private:
	BString		fName;
	BString		fPlace;
	BString		fDescription;
	BString		fCatName;

	time_t		fStart;
};

typedef BObjectList<ReminderEvent> ReminderEventList;*/


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
	void			ReadyToRun();
	

	void			AddEventToList(entry_ref* ref);
	void			ShowEvent(entry_ref* ref);
	int32			CountEvents();
	void			LockEvents() { acquire_sem(fEventLock); };
	void			UnlockEvents() { release_sem(fEventLock); };
	void			Notify() { release_sem(fNotify); };
	static int32	EventLoop(void* data);
	void			ShowEvents();
	void			RefreshEventList();

private:

	Event*				_FileToEvent(entry_ref* ref);
	static int			_CompareFunction(const Event* a, const Event* b);
	
	QueryDBManager*		fDBManager;

	BDirectory*			fEventDir;
	BDirectory*			fTrashDir;
	EventList			fEventList;
	BVolume				fQueryVolume;
	sem_id				fEventLock;
	sem_id				fNotify;
	thread_id			fEventLoop;
	bool				fQuitting;
	BQuery				fQuery;
};

int main();

#endif
