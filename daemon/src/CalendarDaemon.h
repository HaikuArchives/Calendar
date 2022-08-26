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
	void			Pulse();
	void			SetFlags(uint32 mask);

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
