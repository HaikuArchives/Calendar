/*
 * Copyright 2007-2011, Haiku, Inc. All Rights Reserved.
 * Copyright 2020-2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 * Authors:
 *		Julun <host.haiku@gmx.de>
 */

#include "CalendarView.h"

#include <stdlib.h>

#include <CalendarView.h>
#include <DateFormat.h>

#include "ColorConverter.h"
#include "QueryDBManager.h"


CalendarView::CalendarView(
	BRect frame, const char* name, uint32 resizeMask, uint32 flags)
	:
	BCalendarView(frame, name, resizeMask, flags)
{
	_Init();
}


CalendarView::CalendarView(BMessage* archive)
	:
	BCalendarView(archive)
{
	_Init();
}


CalendarView::CalendarView(const char* text)
	:
	BCalendarView(text)
{
	_Init();
}


CalendarView::CalendarView(const char* name, uint32 flags)
	:
	BCalendarView(name, flags)
{
	_Init();
}


void
CalendarView::DrawDay(BView* owner, BRect frame, const char* text,
	bool isSelected, bool isEnabled, bool focus, bool isHighlight)
{
	int drawnYear = Date().Year();
	int drawnMonth = Date().Month();
	int drawnDay = atoi(text);

	// handle dates for (disabled) days prior or following current month
	if (isEnabled == 0 && drawnDay > 15) {
		drawnMonth--;
	}
	if (isEnabled == 0 && drawnDay < 15) {
		drawnMonth++;
	}
	if (drawnMonth < 1) {
		drawnMonth += 12;
		drawnYear--;
	}
	if (drawnMonth > 12) {
		drawnMonth -= 12;
		drawnYear++;
	}
	BDate drawnDate = BDate(Date().Year(), drawnMonth, drawnDay);

	int eventCount
		= fDBManager->GetEventsOfDay(drawnDate, !fMarkHidden)->CountItems();
	if (isEnabled == false && eventCount != 0)
		eventCount = 1;

	rgb_color bgColor = ui_color(B_LIST_BACKGROUND_COLOR);
	rgb_color textColor = ui_color(B_LIST_ITEM_TEXT_COLOR);

	if (isSelected == true) {
		bgColor = ui_color(B_LINK_ACTIVE_COLOR);
		textColor = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
	} else if (eventCount > 0) {
		bgColor = TintColor(bgColor, bgColor, eventCount);
		textColor = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
	}

	if (focus == true && isSelected == false)
		textColor = keyboard_navigation_color();

	if (isEnabled == false)
		textColor = TintColor(textColor, textColor, 2);

	// Tint in case of poor background/text contrast
	float brightDiff = bgColor.Brightness() - textColor.Brightness();
	if (brightDiff < 30 && brightDiff > -30)
		textColor = TintColor(textColor, textColor, 2);

	isHighlight = (eventCount > 0 && isEnabled == true);
	_DrawItem(owner, frame, text, isHighlight, focus, bgColor, textColor);
}


void
CalendarView::SetMarkHidden(bool show)
{
	if (fMarkHidden != show) {
		fMarkHidden = show;
		Invalidate();
	}
}


void
CalendarView::_Init()
{
	fDBManager = new QueryDBManager();
	fMarkHidden = false;
}


void
CalendarView::_DrawItem(BView* owner, BRect frame, const char* text,
	bool isHighlight, bool focus, rgb_color bgColor, rgb_color textColor)
{
	rgb_color lColor = LowColor();
	rgb_color highColor = HighColor();

	SetHighColor(bgColor);
	SetLowColor(bgColor);

	FillRect(frame.InsetByCopy(1.0, 1.0));

	if (focus) {
		rgb_color focusColor = keyboard_navigation_color();
		SetHighColor(focusColor);
		StrokeRect(frame.InsetByCopy(1.0, 1.0));
	}

	SetHighColor(textColor);

	float offsetH = frame.Width() / 2.0;
	float offsetV = frame.Height() / 2.0 + FontHeight(owner) / 2.0 - 2.0;

	BFont font(be_plain_font);
	if (isHighlight)
		font.SetFace(B_BOLD_FACE);
	else
		font.SetFace(B_REGULAR_FACE);
	SetFont(&font);

	DrawString(text,
		BPoint(frame.right - offsetH - StringWidth(text) / 2.0,
			frame.top + offsetV));

	SetLowColor(lColor);
	SetHighColor(highColor);
}


float
FontHeight(const BView* view)
{
	if (!view)
		return 0.0;
	BFont font;
	view->GetFont(&font);
	font_height fheight;
	font.GetHeight(&fheight);
	return ceilf(fheight.ascent + fheight.descent + fheight.leading);
}
