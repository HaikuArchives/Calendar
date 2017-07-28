/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EVENT_H
#define EVENT_H

#include <DateTime.h>
#include <String.h>


class Event {
public:

			Event(const char* name, const char* place,
				const char* description, bool allday,
				BDateTime start, BDateTime end,
				const char* id = NULL);
			Event(Event& event);

	BDateTime	GetStartDateTime();
	void		SetStartDateTime(BDateTime& start);

	BDateTime	GetEndDateTime();
	void		SetEndDateTime(BDateTime& end);


	const char*	GetId();

	const char*	GetName();
	const char*	GetPlace();
	const char*	GetDescription();

	void		SetName(const char* name);
	void		SetPlace(const char* place);
	void		SetDescription(const char* description);

	bool		IsAllDay();
	void		SetAllDay(bool allday);

	bool 		Equals(Event& e);

private:

	BString		fName;
	BString		fDescription;
	BString		fPlace;
	BString		fId;

	BDateTime	fStart;
	BDateTime	fEnd;

	bool		fAllDay;

};


#endif
