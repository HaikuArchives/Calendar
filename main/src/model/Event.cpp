/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Event.h"

#include <Uuid.h>


Event::Event(const char* name, const char* place, const char* description,
	bool allday, time_t start, time_t end, Category* category,
	bool reminded, time_t reminderTime, time_t updated /*=time(NULL)*/,
	uint16 status /*= 0 */, const char* id /*= NULL*/)
{
	fName = name;
	fPlace = place;
	fDescription = description;
	fAllDay = allday;
	fStart = start;
	fEnd = end;
	fUpdated = updated;
	fStatus = status;

	fCategory = new Category(*category);

	fReminded = reminded;
	fReminderTime = reminderTime;

	if (id == NULL)
		fId = BUuid().SetToRandom().ToString();
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
	fStart = event.GetStartDateTime();
	fEnd = event.GetEndDateTime();
	fUpdated = event.GetUpdated();
	fStatus = event.GetStatus();
	fReminded = event.IsReminded();
	fReminderTime = event.GetReminderTime();
}


time_t
Event::GetStartDateTime() const
{
	return fStart;
}


void
Event::SetStartDateTime(time_t start)
{
	fStart = start;
}


time_t
Event::GetEndDateTime() const
{
	return fEnd;
}


void
Event::SetEndDateTime(time_t end)
{

	fEnd = end;
}


const char*
Event::GetId() const
{
	return fId.String();
}


Category*
Event::GetCategory() const
{
	return fCategory;
}


void
Event::SetName(const char* name)
{
	fName = name;
}


const char*
Event::GetName() const
{
	return fName.String();
}


void
Event::SetPlace(const char* place)
{
	fPlace = place;
}


const char*
Event::GetPlace() const
{
	return fPlace.String();
}


void
Event::SetDescription(const char* description)
{
	fDescription = description;
}


const char*
Event::GetDescription() const
{
	return fDescription.String();
}


void
Event::SetAllDay(bool allday)
{
	fAllDay = allday;
}


bool
Event::IsAllDay() const
{
	return fAllDay;
}


uint16
Event::GetStatus() const
{
	return fStatus;
}


void
Event::SetStatus(uint16 status)
{
	fStatus = status;
}


void
Event::SetUpdated(time_t updated)
{
	fUpdated = updated;
}


time_t
Event::GetUpdated() const
{
	return fUpdated;
}


bool
Event::Equals(Event& e) const
{
	return (fId == e.GetId());
}

bool
Event::IsReminded() const
{
	return fReminded;
}

void
Event::SetReminded(bool reminded)
{
	fReminded = reminded;
}

time_t
Event::GetReminderTime() const
{
	return fReminderTime;
}

void
Event::SetReminderTime(time_t reminderTime)
{
	fReminderTime = reminderTime;
}
