/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SidePanelView.h"

#include <Button.h>
#include <Catalog.h>
#include <DateFormat.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>
#include <StringView.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <TextControl.h>

#include "CalendarView.h"
#include "EventTabView.h"
#include "MainView.h"
#include "MainWindow.h"
#include "PreferenceWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SidePanelView"

enum StartOfWeek {
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

	fCalendarView = new CalendarView("calendar");
	fCalendarView->SetWeekNumberHeaderVisible(false);
	// fCalendarView->SetInvocationMessage(new BMessage(kInvokationMessage));
	fCalendarView->SetSelectionMessage(new BMessage(kSelectionMessage));

	fDateHeaderButton = new DateHeaderButton();

	fYearLabel = new BStringView("year", "");
	fMonthLabel = new BStringView("month", "");
	fFilterSearchHeading = new BStringView(BRect(10,10,11,11), "filterSearchHeading", "Filter & Search");
	fFilterSearchHeading->ResizeToPreferred();

	fMonthUpButton
		= new BButton("MonthuUp", "►", new BMessage(kMonthUpMessage));
	fMonthDownButton
		= new BButton("MonthDown", "◄", new BMessage(kMonthDownMessage));

	fMonthUpButton->SetFlat(true);
	fMonthDownButton->SetFlat(true);

	BFont font;
	fMonthLabel->GetFont(&font);
	font.SetSize(font.Size() * 1.1);
	fMonthLabel->SetFont(&font, B_FONT_ALL);

	float width, height;
	fMonthLabel->GetPreferredSize(&width, &height);
	fMonthLabel->SetExplicitMinSize(
		BSize(font.StringWidth(B_TRANSLATE_COMMENT(
				"September XXXX", "Choose the longest month name")),
			height));
			
	fMonthUpButton->SetExplicitMinSize(BSize(height + 5, height + 5));
	fMonthDownButton->SetExplicitMinSize(BSize(height + 5, height + 5));
	
	fProfileMenu = new BPopUpMenu("ProfileMenu");
	fCategoryMenu = new BPopUpMenu("CategoryMenu");
	
	const char * profileMenuItems[] = {B_TRANSLATE("first"),
		B_TRANSLATE("second"), B_TRANSLATE("third") , NULL};
	const char * categoryMenuItems[] = {B_TRANSLATE("first cat"),
		B_TRANSLATE("second cat"), B_TRANSLATE("third cat"), NULL};
	
	static const int tempMessage = 8990;
	
	for(int i=0 ; profileMenuItems[i] ; ++i)
	{
		fProfileMenu->AddItem(new BMenuItem(profileMenuItems[i],
			new BMessage(tempMessage)));
	}
	fProfileMenu->ItemAt(0)->SetMarked(true);
	
	for(int i=0 ; categoryMenuItems[i] ; ++i)
	{
		fCategoryMenu->AddItem(new BMenuItem(categoryMenuItems[i],
			new BMessage(tempMessage)));
	}
	fCategoryMenu->ItemAt(0)->SetMarked(true);
	
	fProfileMenuField = new BMenuField(
		"ProfileMenu", B_TRANSLATE("Profile:"), fProfileMenu);
	fCategoryMenuField = new BMenuField(
		"CategoryMenu", B_TRANSLATE("Category:"), fCategoryMenu);
		
	fTextSearch = new BTextControl("SearchTerms", NULL, NULL, NULL);
	fSearchLabel = new BStringView("SearchLabel", B_TRANSLATE("Search:"));
	
	fClearButton = new BButton(
		B_TRANSLATE("Clear"), new BMessage(tempMessage));
	fClearButton->SetEnabled(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fDateHeaderButton)
		.AddStrut(30)
		.AddGroup(B_HORIZONTAL)
		.Add(fMonthLabel)
		.AddGlue()
		.Add(fMonthDownButton)
		.Add(fMonthUpButton)
		.End()
		.AddStrut(5)
		.Add(fCalendarView)
		.AddStrut(20)
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_SMALL_SPACING)
		.SetInsets(
			B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, 0)
		.Add(fFilterSearchHeading, 0, 0)
		.AddMenuField(fProfileMenuField, 0, 1)
		.AddMenuField(fCategoryMenuField, 0, 2)
		.Add(fSearchLabel, 0, 3)
		.Add(fTextSearch, 1, 3)
		.Add(fClearButton, 1, 4)
		.End()
		.AddGlue(10)
		.End();

	_UpdateDate(BDate::CurrentDate(B_LOCAL_TIME));
}


void
SidePanelView::MessageReceived(BMessage* message)
{
	int32 change;
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
		{
			message->FindInt32(B_OBSERVE_WHAT_CHANGE, &change);
			switch (change) {
				case kSystemDateChangeMessage:
					fDateHeaderButton->MessageReceived(message);
					break;

				default:
					BView::MessageReceived(message);
					break;
			}
			break;
		}
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
			fDateHeaderButton->MessageReceived(message);
			_UpdateDateLabel();
			break;
		}
		case kListModeChanged:
		{
			bool hidden = false;
			if (message->FindBool("hidden", &hidden) == B_OK)
				fCalendarView->SetMarkHidden(hidden);
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
		// Preference menu index to start of week map
	BWeekday firstDay;

	if (startOfWeekDay == kLocaleStartOfWeek) {
		BDateFormat().GetStartOfWeek(&firstDay);
		fCalendarView->SetStartOfWeek(firstDay);
	} else {
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
