/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EVENT_H
#define EVENT_H

#include <time.h>

#include <String.h>

#include "Category.h"


class Event {
public:

			Event(const char* name, const char* place,
				const char* description, bool allday,
				time_t start, time_t end,
				Category* category, bool notified,
				time_t updated = time(NULL), bool status = true,
				const char* id = NULL);
			Event(Event& event);

	time_t		GetStartDateTime();
	void		SetStartDateTime(time_t start);

	time_t		GetEndDateTime();
	void		SetEndDateTime(time_t end);

	const char*	GetId();
	Category*	GetCategory();

	const char*	GetName();
	const char*	GetSanitizedName();
	const char*	GetPlace();
	const char*	GetDescription();

	void		SetName(const char* name);
	void		SetPlace(const char* place);
	void		SetDescription(const char* description);

	bool		IsAllDay();
	void		SetAllDay(bool allday);

	bool		GetStatus();
	void		SetStatus(bool status);

	time_t		GetUpdated();
	void		SetUpdated(time_t updated);


	bool		IsNotified();
	void		SetNotified(bool notified);

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
	bool		fNotified;
	bool		fStatus;

	Category*	fCategory;

};


#endif
