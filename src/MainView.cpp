/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "MainView.h"

#include <DateTime.h>


MainView::MainView()
	:
	BView("MainView", B_WILL_DRAW),
	fMessage(kDateChange),
	fCurrentDate(BDate::CurrentDate(B_LOCAL_TIME))
{
	SetFlags(Flags() | B_PULSE_NEEDED);
}


void
MainView::Pulse()
{
	if (IsWatched())
		_SendNotices();
}


void
MainView::AttachedToWindow()
{
	SetViewColor(255, 255, 255);
}


void
MainView::_SendNotices()
{
	BDate date = BDate::CurrentDate(B_LOCAL_TIME);
	if (fCurrentDate != date) {
		fCurrentDate = date;
		SendNotices(kSystemDateChangeMessage, &fMessage);
	}
}
