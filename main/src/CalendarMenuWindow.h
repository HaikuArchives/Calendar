/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2008 Karsten Heimrich, host.haiku@gmx.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CALENDAR_MENU_WINDOW_H_
#define _CALENDAR_MENU_WINDOW_H_


#include <DateTime.h>
#include <Window.h>


class BMessage;
class BStringView;

namespace BPrivate {
class BCalendarView;
}

using BPrivate::BCalendarView;


class CalendarMenuWindow : public BWindow
{
public:
					CalendarMenuWindow(BHandler* handler, BPoint where);
	virtual 		~CalendarMenuWindow();

	virtual void 	Show();
	virtual void 	WindowActivated(bool active);
	virtual void 	MessageReceived(BMessage* message);
	void 			SetDate(const BDate& date);
	void 			SetInvocationMessage(BMessage* message);

private:
	void 			_UpdateDate(const BDate& date);
	BButton* 		_SetupButton(const char* label, uint32 what, float height);

private:
	static const int kInvokationMessage = 1000;
	static const int kMonthDownMessage = 1001;
	static const int kMonthUpMessage = 1002;
	static const int kYearDownMessage = 1003;
	static const int kYearUpMessage = 1004;


	BStringView*	fYearLabel;
	BStringView*	fMonthLabel;
	BCalendarView*	fCalendarView;
	BHandler*		fHandler;
	BMessage*		fInvocationMessage;
	bool			fSuppressFirstClose;
};


#endif /* _CALENDAR_MENU_WINDOW_H_ */
