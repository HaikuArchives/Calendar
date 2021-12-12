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
					CalendarView(BRect frame, const char* name,
						uint32 resizeMask, uint32 flags);
					CalendarView(BMessage* archive);
					CalendarView(const char* text);
					CalendarView(const char* name, uint32 flags);

	virtual	void	DrawDay(BView* owner, BRect frame, const char* text,
						bool isSelected, bool isEnabled, bool focus,
						bool isHighlight);

			void	SetMarkHidden(bool show);

private:
			void	_Init();
			void	_DrawItem(BView* owner, BRect frame, const char* text,
						bool isHighlight, bool focus, rgb_color bgColor,
						rgb_color textColor);

		bool fMarkHidden;
		QueryDBManager*	fDBManager;
};


float		FontHeight(const BView* view);


#endif
