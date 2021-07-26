/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Event.h"

#include <regex>
#include <Uuid.h>


Event::Event(const char* name,
	const char* place, const char* description,
	bool allday, time_t start, time_t end,
	Category* category, bool notified,
	time_t updated /*=time(NULL)*/, bool status /*= false*/,
	const char* id /*= NULL*/)
{
	fName = name;
	fPlace = place;
	fDescription = description;
	fAllDay = allday;
	fNotified = notified;
	fStart = start;
	fEnd = end;
	fUpdated = updated;
	fStatus = status;

	fCategory = new Category(*category);

	if (id == NULL) {
		fId = BUuid().SetToRandom().ToString();
	}
	else
		fId = id;

}


Event::Event(Event& event)
{
	fName = event.GetName();
	fPlace = event.GetPlace();
	fId = event.GetId();
	fCategory = event.GetCategory();
	fDescription = event.GetDescription();
	fAllDay = event.IsAllDay();
	fNotified = event.IsNotified();
	fStart = event.GetStartDateTime();
	fEnd = event.GetEndDateTime();
	fUpdated = event.GetUpdated();
	fStatus = event.GetStatus();
}


time_t
Event::GetStartDateTime()
{
	return fStart;
}


void
Event::SetStartDateTime(time_t start)
{
	fStart = start;
}


time_t
Event::GetEndDateTime()
{
	return fEnd;
}


void
Event::SetEndDateTime(time_t end)
{

	fEnd = end;
}


const char*
Event::GetId()
{
	return fId.String();
}


Category*
Event::GetCategory()
{
	return fCategory;
}


void
Event::SetName(const char* name)
{
	fName = name;
}


const char*
Event::GetName()
{
	return fName.String();
}

const char*
Event::GetSanitizedName()
{
	std::regex expression("[^\\w\\d]");
	return std::regex_replace(fName.String(), expression, "_").c_str();
}

void
Event::SetPlace(const char* place)
{
	fPlace = place;
}


const char*
Event::GetPlace()
{
	return fPlace.String();
}

void
Event::SetDescription(const char* description)
{
	fDescription = description;
}


const char*
Event::GetDescription()
{
	return fDescription.String();
}


void
Event::SetAllDay(bool allday)
{
	fAllDay = allday;
}


bool
Event::IsAllDay()
{
	return fAllDay;
}


void
Event::SetNotified(bool notified)
{
	fNotified = notified;
}


bool
Event::IsNotified()
{
	return fNotified;
}



bool
Event::GetStatus()
{
	return fStatus;
}


void
Event::SetStatus(bool status)
{
	fStatus = status;
}


void
Event::SetUpdated(time_t updated)
{
	fUpdated = updated;
}


time_t
Event::GetUpdated()
{
	return fUpdated;
}


bool
Event::Equals(Event &e)
{
    return (fId == e.GetId());
}

