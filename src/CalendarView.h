/*
 * Copyright 2007-2011, Haiku, Inc. All Rights Reserved.
 * Copyright 2020-2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 * Authors:
 *		Julun <host.haiku@gmx.de>
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
		void			_DrawItem(BView* owner, BRect frame, const char* text,
							bool isHighlight, bool focus, rgb_color bgColor,
							rgb_color textColor);

		QueryDBManager*	fDBManager;
};


rgb_color	TintColor(rgb_color color, rgb_color base, int severity);
float		FontHeight(const BView* view);


#endif
