/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef SIDE_PANEL_VIEW_H
#define SIDE_PANEL_VIEW_H

#include <Button.h>
#include <DateTime.h>
#include <CalendarView.h>
#include <StringView.h>
#include <View.h>

#include "DateHeaderView.h"

using BPrivate::BCalendarView;


enum {
	kSelectionMessage,
	kMonthDownMessage,
	kMonthUpMessage,
	kShowWeekNumberMessage,
	kSetStartOfWeekMessage,
	kSetCalendarToCurrentDate,
};


class SidePanelView: public BView {
public:
					SidePanelView();

		void			MessageReceived(BMessage* message);
		BDate			GetSelectedDate() const;
		void	 		SetStartOfWeek(int32);
		void			ShowWeekHeader(bool);

private:

		void 			_UpdateDate(const BDate&);
		void			_UpdateDateLabel();

		BStringView*		fYearLabel;
		BStringView*		fMonthLabel;
		BCalendarView*		fCalendarView;
		DateHeaderView*		fDateHeaderView;
		BButton*		fMonthUpButton;
		BButton*		fMonthDownButton;
};

#endif
