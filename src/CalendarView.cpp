/*
 * Copyright 2017 Jadedctrl
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CalendarView.h"

#include <CalendarView.h>
#include <DateFormat.h>
#include <stdlib.h>

#include "QueryDBManager.h"


CalendarView::CalendarView(BRect frame, const char* name,
	uint32 resizeMask, uint32 flags)
	:
	BCalendarView(frame, name, resizeMask, flags)
{
	_Init();
}

CalendarView::CalendarView (const char* text)
	:
	BCalendarView(text)
{
	_Init();
}

CalendarView::CalendarView (BMessage* archive)
	:
	BCalendarView(archive)
{
	_Init();
}

CalendarView::CalendarView (const char* name, uint32 flags)
	:
	BCalendarView(name, flags)
{
	_Init();
}

void
CalendarView::_Init ()
{
	fDBManager = new QueryDBManager();
}


void
CalendarView::DrawDay (BView* owner, BRect frame, const char* text,
	bool isSelected, bool isEnabled, bool focus, bool highlight)
{
	int drawnYear  = Date().Year();
	int drawnMonth = Date().Month();
	int drawnDay   = atoi(text);

	// handle dates for (disabled) days prior or following current month
	if (isEnabled == 0 && drawnDay > 15)  { drawnMonth--; }
	if (isEnabled == 0 && drawnDay < 15)  { drawnMonth++; }
	if (drawnMonth < 1)  { drawnMonth += 12; drawnYear--; }
	if (drawnMonth > 12) { drawnMonth -= 12; drawnYear++; }

	BDate drawnDate = BDate(Date().Year(), drawnMonth, drawnDay);
	int currentEventsCount = fDBManager->GetEventsOfDay(drawnDate)->CountItems();

	if (currentEventsCount > 0) { highlight = true; }

	BCalendarView::DrawDay(owner, frame, text, isSelected, isEnabled, focus, highlight);
}
