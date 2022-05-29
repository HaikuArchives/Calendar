/*
 * Copyright 2004-2011, Haiku, Inc. All Rights Reserved.
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2022, Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Axel Dörfler <axeld@pinc-software.de>
 *		Hamish Morrison <hamish@lavabit.com>
 *		Julun <host.haiku@gmx.de>
 *		Mike Berg <mike@berg-net.us>
 */

#include "DateTimeEdit.h"

#include <ControlLook.h>

#include "CalendarMenuWindow.h"


const uint32 kArrowAreaWidth = 16;
const int32 kCalendarMenuDate = 'cald';


DateEdit::DateEdit(const char* name, BMessage* message)
	:
	BPrivate::DateEdit(name, 3, message)
{
}


void
DateEdit::MessageReceived(BMessage* message)
{
	if (message->what == kCalendarMenuDate) {
		int32 day, month, year;
		if (message->FindInt32("day", &day) == B_OK
			&& message->FindInt32("month", &month) == B_OK
			&& message->FindInt32("year", &year) == B_OK)
			SetDate(year, month, day);
	} else
		BPrivate::DateEdit::MessageReceived(message);
}


void
DateEdit::MouseDown(BPoint where)
{
	if (fCalendarButton.Contains(where) && IsEnabled()) {
		if (fCalendarWindow.IsValid()) {
			BMessage activate(B_SET_PROPERTY);
			activate.AddSpecifier("Active");
			activate.AddBool("data", true);
			if (fCalendarWindow.SendMessage(&activate) == B_OK)
				return;
		}

		CalendarMenuWindow* window =
			new CalendarMenuWindow(this, ConvertToScreen(fCalendarPoint));
		window->SetInvocationMessage(new BMessage(kCalendarMenuDate));
		window->SetDate(GetDate());
		fCalendarWindow = BMessenger(window);

		BPoint centeredCalendarPt = ConvertToScreen(fCalendarPoint);
		centeredCalendarPt.x -= window->Frame().Width();
		window->MoveTo(centeredCalendarPt);
		window->Show();
	} else
		BPrivate::DateEdit::MouseDown(where);
}


void
DateEdit::Draw(BRect updateRect)
{
	if (IsEnabled())
		SetViewUIColor(B_DOCUMENT_BACKGROUND_COLOR);
	else
		SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	SetLowColor(ViewColor());
	FillRect(updateRect, B_SOLID_LOW);
	BPrivate::DateEdit::Draw(updateRect);
}


void
DateEdit::DrawBorder(const BRect& updateRect)
{
	BRect bounds(Bounds());
	bool showFocus = (IsFocus() && Window() && Window()->IsActive());

	be_control_look->DrawBorder(this, bounds, updateRect, ViewColor(),
		B_FANCY_BORDER, showFocus ? BControlLook::B_FOCUSED : 0);

	// Instead of the up-down arrows, draw a "this opens something up" arrow
	bounds.left = bounds.right - kArrowAreaWidth;
	bounds.right = Bounds().right - 2;
	if (!updateRect.Intersects(bounds))
		return;

	const float vertMargin = 6;
	const float horizMargin = 3;
	BPoint left(bounds.left + horizMargin, bounds.top + vertMargin);
	BPoint right(bounds.right - horizMargin, bounds.top + vertMargin);
	BPoint middle(bounds.left + floorf(bounds.Width() / 2), bounds.bottom - vertMargin);

	fCalendarButton = bounds;
	fCalendarPoint = middle;

	FillRect(bounds, B_SOLID_LOW);
	FillTriangle(left, right, middle, B_SOLID_HIGH);
}


TimeEdit::TimeEdit(const char* name, BMessage* message)
	:
	BPrivate::TimeEdit(name, 3, message)
{
	BFormattingConventions conventions;
	BLocale().GetFormattingConventions(&conventions);
	if (conventions.Use24HourClock() == false)
		fSectionCount = 4;
}


void
TimeEdit::Draw(BRect updateRect)
{
	if (IsEnabled())
		SetViewUIColor(B_DOCUMENT_BACKGROUND_COLOR);
	else
		SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	SetLowColor(ViewColor());
	FillRect(updateRect, B_SOLID_LOW);
	BPrivate::TimeEdit::Draw(updateRect);
}
