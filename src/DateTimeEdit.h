/*
 * Copyright 2004-2011, Haiku, Inc. All Rights Reserved.
 * Copyright 2022, Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef DATE_EDIT_H
#define DATE_EDIT_H

#include <DateTimeEdit.h>
#include <Messenger.h>


class DateEdit : public BPrivate::DateEdit {
public:
					DateEdit(const char* name, BMessage* message = NULL);

	virtual void	MessageReceived(BMessage* message);

	virtual void	MouseDown(BPoint where);

	virtual void	Draw(BRect updateRect);

protected:
	virtual void	DrawBorder(const BRect& updateRect);

private:
	BMessenger fCalendarWindow;
	BRect fCalendarButton;
	BPoint fCalendarPoint;
};


class TimeEdit : public BPrivate::TimeEdit {
public:
					TimeEdit(const char* name, BMessage* message = NULL);

	virtual void	Draw(BRect updateRect);
};

#endif // DATE_EDIT_H
