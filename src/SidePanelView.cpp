/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "SidePanelView.h"

#include <Button.h>
#include <DateFormat.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>
#include <StringView.h>

#include "MainView.h"
#include "MainWindow.h"
#include "PreferenceWindow.h"
#include "OurCalendarView.h"


enum StartOfWeek
{
	kLocaleStartOfWeek,
	kWeekDayMonday,
	kWeekDayTuesday,
	kWeekDayWednesday,
	kWeekDayThursday,
	kWeekDayFriday,
	kWeekDaySaturday,
	kWeekDaySunday,
};


SidePanelView::SidePanelView()
	:
	BView("SidePanelView", B_WILL_DRAW)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fCalendarView = new OurCalendarView("calendar");
	fCalendarView->SetWeekNumberHeaderVisible(false);
	//fCalendarView->SetInvocationMessage(new BMessage(kInvokationMessage));
	fCalendarView->SetSelectionMessage(new BMessage(kSelectionMessage));

	fDateHeaderView = new DateHeaderView();

	fYearLabel = new BStringView("year", "");
	fMonthLabel = new BStringView("month", "");

	fMonthUpButton = new BButton("MonthuUp", "►", new BMessage(kMonthUpMessage));
	fMonthDownButton = new BButton("MonthDown", "◄", new BMessage(kMonthDownMessage));

	fMonthUpButton->SetFlat(true);
	fMonthDownButton->SetFlat(true);

	BFont font;
	fMonthLabel->GetFont(&font);
	font.SetSize(font.Size() * 1.1);
	fMonthLabel->SetFont(&font, B_FONT_ALL);

	float width, height;
	fMonthLabel->GetPreferredSize(&width, &height);
	fMonthLabel->SetExplicitMinSize(BSize(font.StringWidth("September XXXX"),
		height));

	fMonthUpButton->SetExplicitMinSize(BSize(height + 5, height + 5));
	fMonthDownButton->SetExplicitMinSize(BSize(height + 5, height + 5));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.SetInsets(15)
		.Add(fDateHeaderView)
		.AddStrut(30)
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.SetInsets(5.0, 5.0, 5.0, 5.0)
			.Add(fMonthLabel)
			.AddGlue()
			.Add(fMonthDownButton)
			.Add(fMonthUpButton)
		.End()
		.AddStrut(5)
		.Add(fCalendarView)
		.AddGlue()
	.End();

	_UpdateDate(BDate::CurrentDate(B_LOCAL_TIME));
}


void
SidePanelView::MessageReceived(BMessage* message)
{
	int32 change;
	switch (message->what) {

		case B_OBSERVER_NOTICE_CHANGE:
			message->FindInt32(B_OBSERVE_WHAT_CHANGE, &change);
			switch (change) {
				case kSystemDateChangeMessage:
					fDateHeaderView->MessageReceived(message);
					break;

				default:
					BView::MessageReceived(message);
					break;
			}
			break;

		case kSelectionMessage:
		{
			int32 day, month, year;
			message->FindInt32("day", &day);
			message->FindInt32("month", &month);
			message->FindInt32("year", &year);

			_UpdateDate(BDate(year, month, day));
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

		case B_LOCALE_CHANGED:
		{
			BLocaleRoster::Default()->Refresh();
			fDateHeaderView->MessageReceived(message);
			_UpdateDateLabel();
			break;
		}

		case kSetCalendarToCurrentDate:
			_UpdateDate(BDate::CurrentDate(B_LOCAL_TIME));
			break;

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
SidePanelView::SetStartOfWeek(int32 index)
{
	StartOfWeek startOfWeekDay = static_cast<StartOfWeek>(index);
		//Preference menu index to start of week map
	BWeekday firstDay;

	if (startOfWeekDay == kLocaleStartOfWeek) {
		BDateFormat().GetStartOfWeek(&firstDay);
		fCalendarView->SetStartOfWeek(firstDay);
	}

	else
	{
		firstDay = static_cast<BWeekday>(index);
		fCalendarView->SetStartOfWeek(firstDay);
	}
}


void
SidePanelView::ShowWeekHeader(bool state)
{
	fCalendarView->SetWeekNumberHeaderVisible(state);
}


void
SidePanelView::_UpdateDate(const BDate& date)
{
	if (!date.IsValid())
		return;

	fCalendarView->SetDate(date);
	_UpdateDateLabel();

	Window()->PostMessage(kSelectedDateChanged);
}


BDate
SidePanelView::GetSelectedDate() const
{
	return fCalendarView->Date();
}


void
SidePanelView::_UpdateDateLabel()
{
	BDate date = fCalendarView->Date();

	BString yearString;
	BString monthYearString;
	yearString << date.Year();

	BDateFormat().GetMonthName(date.Month(), monthYearString);
	monthYearString += " ";
	monthYearString += yearString.String();

	fMonthLabel->SetText(monthYearString);
}
