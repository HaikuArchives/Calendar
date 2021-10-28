/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EVENT_H
#define EVENT_H

#include <time.h>

#include <String.h>

#include "Category.h"


enum {
	EVENT_NOTIFIED		= 1,
	EVENT_CANCELLED		= 2,
	EVENT_DELETED		= 4,
	EVENT_HIDDEN		= 8
};


class Event {
public:

			Event(const char* name, const char* place,
				const char* description, bool allday,
				time_t start, time_t end,
				Category* category,
				time_t updated = time(NULL), uint16 status = 0,
				const char* id = NULL);
			Event(Event& event);

	time_t		GetStartDateTime();
	void		SetStartDateTime(time_t start);

	time_t		GetEndDateTime();
	void		SetEndDateTime(time_t end);

	const char*	GetId();
	Category*	GetCategory();

	const char*	GetName();
	const char*	GetPlace();
	const char*	GetDescription();

	void		SetName(const char* name);
	void		SetPlace(const char* place);
	void		SetDescription(const char* description);

	bool		IsAllDay();
	void		SetAllDay(bool allday);

	uint16		GetStatus();
	void		SetStatus(uint16 status);

	time_t		GetUpdated();
	void		SetUpdated(time_t updated);

	bool 		Equals(Event& e);

private:
	BString		fName;
	BString		fDescription;
	BString		fPlace;
	BString		fId;

	time_t		fStart;
	time_t		fEnd;
	time_t		fUpdated;

	bool		fAllDay;
	int16		fStatus;

	Category*	fCategory;

};


#endif
