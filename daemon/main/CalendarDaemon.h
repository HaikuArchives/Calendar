/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef CALENDAR_DAEMON_H
#define CALENDAR_DAEMON_H

#include <Application.h>

#include "Event.h"

enum
{
	kEventAdded,
	kEventRemoved,
	kEventChanged,
	kEventNotify,
};

class BMessage;
class BString;


/*!
	ReminderEvent Class Declaration
*/

class ReminderEvent
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

typedef BObjectList<ReminderEvent> ReminderEventList;


/*!
	CalendarDaemon Class Declaration
*/

class CalendarDaemon : public BApplication
{
public:
		CalendarDaemon();
		~CalendarDaemon();
	
	void		MessageReceived(BMessage* message);
	bool		QuitRequested();
	
	void		Notify(Event* event);
	void		AddEventToList(entry_ref* ref);
	void		ShowEvent(entry_ref* ref);

private:

	ReminderEvent*		_FileToReminderEvent(entry_ref* ref);
	int					_CompareFunction(ReminderEvent* a, ReminderEvent* b);

	BString				fEventDir;
	ReminderEventList*	fEventList;

};

int main();

#endif
