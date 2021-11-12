/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "DateHeaderButton.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <DateFormat.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>
#include <StringView.h>

#include "MainView.h"
#include "SidePanelView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DateHeaderButton"


DateHeaderButton::DateHeaderButton()
	: BControl("DateHeaderButton", NULL,
		new BMessage(kSetCalendarToCurrentDate), B_WILL_DRAW)
{
	fMouseInView = false;
	fMouseDown = false;

	SetToolTip(B_TRANSLATE("Go to today"));
	SetExplicitMinSize(BSize(B_SIZE_UNSET, be_plain_font->Size() * 4.4));
	_UpdateDateHeader();
}


void
DateHeaderButton::Draw(BRect updateRect)
{
	BRect bounds = Bounds();

	if (fMouseInView == true && IsEnabled() == true) {
		BRect buttonBound = bounds;
		int32 flags = BControlLook::B_HOVER;
		if (fMouseDown == true)
			flags = BControlLook::B_ACTIVATED;

		be_control_look->DrawButtonFrame(this, buttonBound, updateRect,
			LowColor(), ViewColor(), flags);
		be_control_look->DrawButtonBackground(this, buttonBound, updateRect,
			LowColor(), flags);

		if (fMouseDown == true)
			bounds = buttonBound;
		// DrawButtonBackground will apply the proper inset if button is
		// activated
	}

	float spacing = be_control_look->DefaultLabelSpacing();

	float dayFontSize = be_plain_font->Size() * 3.6;
	float dayTextTop = (bounds.top / 2) + (dayFontSize / 4);
	float dayTextBottom = (bounds.top / 2) + dayFontSize;
	float dayOffset_x = (bounds.left / 2) + spacing;

	BFont font;
	font.SetSize(dayFontSize);
	SetFont(&font);
	MovePenTo(dayOffset_x, dayTextBottom);
	DrawString(fDayText);

	float monthWeekOffset_x = dayOffset_x + font.StringWidth(fDayText) + spacing;

	font.SetSize(be_plain_font->Size() * 1.2);
	SetFont(&font);
	MovePenTo(monthWeekOffset_x, dayTextTop + font.Size());
	DrawString(fDayOfWeekText);

	font.SetSize(be_plain_font->Size());
	SetFont(&font);
	MovePenTo(monthWeekOffset_x, dayTextBottom);
	DrawString(fMonthYearText);
}


void
DateHeaderButton::MessageReceived(BMessage* message)
{
	int32 change;
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
		{	message->FindInt32(B_OBSERVE_WHAT_CHANGE, &change);
			switch (change) {
				case kSystemDateChangeMessage:
					_UpdateDateHeader();
					break;

				default:
					BView::MessageReceived(message);
					break;
			}
			break;
		}
		case B_LOCALE_CHANGED:
			_UpdateDateHeader();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}


void
DateHeaderButton::MouseDown(BPoint where)
{
	if (fMouseInView == true && fMouseDown == false) {
		fMouseDown = true;
		Invalidate();
	}
}


void
DateHeaderButton::MouseUp(BPoint where)
{
	if (fMouseDown == true && IsEnabled() == true) {
		Invoke();
		Invalidate();
	}
	fMouseDown = false;
}


void
DateHeaderButton::MouseMoved(BPoint where, uint32 code, const BMessage* drag)
{
	bool inView = (Bounds().Contains(where) == true);
	if (inView != fMouseInView) {
		// Toggle fMouseDown if button was released since last in view
		if (fMouseDown == true && Window() != NULL
			&& Window()->CurrentMessage() != NULL)
			fMouseDown = (Window()->CurrentMessage()->FindInt32("buttons") != 0);

		fMouseInView = inView;
		Invalidate();
	}
}


void
DateHeaderButton::_UpdateDateHeader()
{
	time_t timeValue = (time_t)time(NULL);

	BString dateString;
	BString dayString;
	BString dayOfWeekString;
	BString monthString;
	BString yearString;
	BString monthYearString;

	int* fieldPositions;
	int positionCount;
	BDateElement* fields;
	int fieldCount;
	BDateFormat dateFormat;

	dateFormat.Format(dateString, fieldPositions, positionCount,
		timeValue, B_FULL_DATE_FORMAT);
	dateFormat.GetFields(fields, fieldCount, B_FULL_DATE_FORMAT);

	for (int i = 0; i < fieldCount; ++i)  {
		if (fields[i] == B_DATE_ELEMENT_WEEKDAY)
			dateString.CopyCharsInto(dayOfWeekString, fieldPositions[i * 2],
				fieldPositions[i * 2 + 1] - fieldPositions[i * 2]);
		else if (fields[i] == B_DATE_ELEMENT_DAY)
			dateString.CopyCharsInto(dayString, fieldPositions[i * 2],
				fieldPositions[i * 2 + 1] - fieldPositions[i * 2]);
		else if (fields[i] == B_DATE_ELEMENT_MONTH)
			dateString.CopyCharsInto(monthString, fieldPositions[i * 2],
				fieldPositions[i * 2 + 1] - fieldPositions[i * 2]);
		else if (fields[i] == B_DATE_ELEMENT_YEAR)
			dateString.CopyCharsInto(yearString, fieldPositions[i * 2],
				fieldPositions[i * 2 + 1] - fieldPositions[i * 2]);
	}

	monthYearString.AppendChars(monthString, monthString.CountChars());
	monthYearString += " ";
	monthYearString.AppendChars(yearString, yearString.CountChars());

	fDayOfWeekText = dayOfWeekString;
	fDayText = dayString;
	fMonthYearText = monthYearString;
}
