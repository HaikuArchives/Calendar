/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef SIDE_PANEL_VIEW_H
#define SIDE_PANEL_VIEW_H

#include <DateTime.h>
#include <View.h>
#include <Window.h>

#include "CalendarView.h"
#include "DateHeaderButton.h"

class BButton;
class BStringView;
class BPopUpMenu;
class BMenuField;
class BTextControl;

enum {
	kSelectionMessage,
	kMonthDownMessage,
	kMonthUpMessage,
	kShowWeekNumberMessage,
	kSetStartOfWeekMessage,
	kSetCalendarToCurrentDate,
	kSelectedDateChanged,
};


class SidePanelView : public BView
{
public:
					SidePanelView();

	void			MessageReceived(BMessage* message);
	BDate			GetSelectedDate() const;
	void			SetStartOfWeek(int32);
	void			ShowWeekHeader(bool);

private:
	void			_UpdateDate(const BDate&);
	void			_UpdateDateLabel();

	BStringView*	fYearLabel;
	BStringView*	fMonthLabel;
	BStringView*	fFilterSearchHeading;
	CalendarView*	fCalendarView;
	DateHeaderButton* fDateHeaderButton;
	BButton*		fMonthUpButton;
	BButton*		fMonthDownButton;
	
	BPopUpMenu*		fProfileMenu;
	BPopUpMenu*		fCategoryMenu;
	BMenuField*		fProfileMenuField;
	BMenuField*		fCategoryMenuField;
	
	BTextControl*	fTextSearch;
	BStringView*	fSearchLabel;
	
	BButton*		fClearButton;
};

#endif
