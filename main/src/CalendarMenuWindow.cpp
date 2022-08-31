/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2008 Karsten Heimrich, host.haiku@gmx.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */


#include "CalendarMenuWindow.h"

#include <Button.h>
#include <CalendarView.h>
#include <DateFormat.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <GroupView.h>
#include <Locale.h>
#include <Screen.h>
#include <SpaceLayoutItem.h>
#include <String.h>
#include <StringView.h>

#include "EventWindow.h"


using BPrivate::BCalendarView;


//	#pragma mark - FlatButton


class FlatButton : public BButton
{
public:
	FlatButton(const BString& label, uint32 what)
		:
		BButton(label.String(), new BMessage(what))
	{
	}
	virtual ~FlatButton() {}

	virtual void Draw(BRect updateRect);
};


void
FlatButton::Draw(BRect updateRect)
{
	updateRect = Bounds();
	rgb_color highColor = HighColor();

	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(updateRect);

	font_height fh;
	GetFontHeight(&fh);

	const char* label = Label();
	const float stringWidth = StringWidth(label);
	const float x = (updateRect.right - stringWidth) / 2.0f;
	const float labelY = updateRect.top
		+ ((updateRect.Height() - fh.ascent - fh.descent) / 2.0f) + fh.ascent
		+ 1.0f;

	SetHighColor(highColor);
	DrawString(label, BPoint(x, labelY));

	if (IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(updateRect);
	}
}


//	#pragma mark - CalendarMenuWindow


CalendarMenuWindow::CalendarMenuWindow(BHandler* handler, BPoint where)
	:
	BWindow(BRect(0.0, 0.0, 100.0, 130.0), "", B_BORDERED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS | B_CLOSE_ON_ESCAPE
			| B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE),
	fYearLabel(NULL),
	fMonthLabel(NULL),
	fHandler(handler),
	fInvocationMessage(NULL),
	fCalendarView(NULL),
	fSuppressFirstClose(true)
{
	SetFeel(B_FLOATING_ALL_WINDOW_FEEL);

	RemoveShortcut('H', B_COMMAND_KEY | B_CONTROL_KEY);
	AddShortcut('W', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));

	fYearLabel = new BStringView("year", "");
	fMonthLabel = new BStringView("month", "");

	fCalendarView = new BCalendarView(Bounds(), "calendar", B_FOLLOW_ALL);
	fCalendarView->SetInvocationMessage(new BMessage(kInvokationMessage));

	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	float width, height;
	fMonthLabel->GetPreferredSize(&width, &height);

	BGridLayout* gridLayout = BGridLayoutBuilder(5.0)
	  .Add(_SetupButton("-", kMonthDownMessage, height), 0, 0)
	  .Add(fMonthLabel, 1, 0)
	  .Add(_SetupButton("+", kMonthUpMessage, height), 2, 0)
	  .Add(BSpaceLayoutItem::CreateGlue(), 3, 0)
	  .Add(_SetupButton("-", kYearDownMessage, height), 4, 0)
	  .Add(fYearLabel, 5, 0)
	  .Add(_SetupButton("+", kYearUpMessage, height), 6, 0)
	  .SetInsets(5.0, 0.0, 5.0, 0.0);
	gridLayout->SetMinColumnWidth(1, be_plain_font->StringWidth("September"));

	BGroupView* groupView = new BGroupView(B_VERTICAL, 10.0);
	BView* view = BGroupLayoutBuilder(B_VERTICAL, 5.0)
	  .Add(gridLayout->View())
	  .Add(fCalendarView)
	  .SetInsets(5.0, 5.0, 5.0, 5.0)
	  .TopView();
	groupView->AddChild(view);
	AddChild(groupView);

	MoveTo(where);
}


CalendarMenuWindow::~CalendarMenuWindow()
{
	SetInvocationMessage(NULL);
}


void
CalendarMenuWindow::Show()
{
	fCalendarView->MakeFocus(true);
	BWindow::Show();
}


void
CalendarMenuWindow::WindowActivated(bool active)
{
	if (active)
		return;

	if (mouse_mode() != B_FOCUS_FOLLOWS_MOUSE) {
		if (!active)
			PostMessage(B_QUIT_REQUESTED);
	} else {
		if (fSuppressFirstClose && !active) {
			fSuppressFirstClose = false;
			return;
		}

		if (!fSuppressFirstClose && !active)
			PostMessage(B_QUIT_REQUESTED);
	}
}


void
CalendarMenuWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kInvokationMessage:
		{
			int32 day, month, year;
			message->FindInt32("day", &day);
			message->FindInt32("month", &month);
			message->FindInt32("year", &year);
			BDate date = BDate(year, month, day);
			_UpdateDate(date);

			BMessenger msgr(fHandler);
			fInvocationMessage->AddInt32("day", day);
			fInvocationMessage->AddInt32("month", month);
			fInvocationMessage->AddInt32("year", year);
			msgr.SendMessage(fInvocationMessage);

			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		case kMonthDownMessage:
		case kMonthUpMessage:
		{
			BDate date = fCalendarView->Date();
			date.AddMonths(kMonthDownMessage == message->what ? -1 : 1);
			_UpdateDate(date);
			break;
		}

		case kYearDownMessage:
		case kYearUpMessage:
		{
			BDate date = fCalendarView->Date();
			date.AddYears(kYearDownMessage == message->what ? -1 : 1);
			_UpdateDate(date);
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
CalendarMenuWindow::SetInvocationMessage(BMessage* message)
{
	delete fInvocationMessage;
	fInvocationMessage = message;
}


void
CalendarMenuWindow::SetDate(const BDate& date)
{
	_UpdateDate(date);
}


void
CalendarMenuWindow::_UpdateDate(const BDate& date)
{
	if (!date.IsValid())
		return;

	fCalendarView->SetDate(date);

	BString yearString;
	BString monthString;

	yearString << date.Year();
	fYearLabel->SetText(yearString);

	BDateFormat().GetMonthName(date.Month(), monthString);
	fMonthLabel->SetText(monthString);
}


BButton*
CalendarMenuWindow::_SetupButton(const char* label, uint32 what, float height)
{
	FlatButton* button = new FlatButton(label, what);
	button->SetExplicitMinSize(BSize(height, height));

	return button;
}
