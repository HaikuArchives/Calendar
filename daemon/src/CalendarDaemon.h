/*
 * Copyright 2022, Harshit Sharma <harshits908@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef CALENDAR_DAEMON_H
#define CALENDAR_DAEMON_H

#include <Application.h>
#include <OS.h>
#include <Volume.h>


/*!
	CalendarDaemon Class Declaration
*/


class CalendarDaemon : public BApplication
{
public:
		CalendarDaemon();
		~CalendarDaemon();

	void			MessageReceived(BMessage* message);
	bool			QuitRequested();

private:

	BString				fEventDir;
	BVolume				fQueryVolume;
};

#endif
