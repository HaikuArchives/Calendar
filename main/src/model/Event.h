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
	EVENT_NOTIFIED = 1,
	EVENT_CANCELLED = 2,
	EVENT_DELETED = 4,
	EVENT_HIDDEN = 8
};


class Event
{
public:
				Event(const char* name, const char* place, const char* description,
					bool allday, time_t start, time_t end, Category* category,
					time_t updated = time(NULL), uint16 status = 0, const char* id = NULL);
				Event(Event& event);

	time_t		GetStartDateTime() const;
	void		SetStartDateTime(time_t start);

	time_t		GetEndDateTime() const;
	void		SetEndDateTime(time_t end);

	const char*	GetId() const;
	Category*	GetCategory() const;

	const char*	GetName() const;
	const char*	GetPlace() const;
	const char*	GetDescription() const;

	void		SetName(const char* name);
	void		SetPlace(const char* place);
	void		SetDescription(const char* description);

	bool		IsAllDay() const;
	void		SetAllDay(bool allday);

	uint16		GetStatus() const;
	void		SetStatus(uint16 status);

	time_t		GetUpdated() const;
	void		SetUpdated(time_t updated);

	bool		Equals(Event& e) const;

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


typedef BObjectList<Event> EventList;

#endif
