/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EVENT_H
#define EVENT_H

#include <DateTime.h>
#include <String.h>

#include "Category.h"


class Event {
public:

			Event(const char* name, const char* place,
				const char* description, bool allday,
				time_t start, time_t end,
				Category* category, bool notified, const char* id = NULL);
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

	bool		fAllDay;
	bool		fNotified;

	Category*	fCategory;

};


#endif
