/*
 * Copyright 2017 Jadedctrl
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CALENDAR_VIEW_H
#define CALENDAR_VIEW_H

#include <CalendarView.h>
#include <DateTime.h>
#include <View.h>

#include "QueryDBManager.h"


namespace BPrivate {
	class BCalendarView;
}

using BPrivate::BCalendarView;


class CalendarView: public BCalendarView {
public:
					CalendarView(BRect, const char*, uint32, uint32);
					CalendarView(BMessage*);
					CalendarView(const char*);
					CalendarView(const char*, uint32);

		void			DrawDay(BView*, BRect, const char*, bool, bool, bool, bool);
private:
		void			_Init();
		QueryDBManager*	fDBManager;
};

#endif
