/*
 * Copyright 2022, Harshit Sharma <harshits908@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef CALENDAR_DAEMON_H
#define CALENDAR_DAEMON_H

#include <iostream>

#include <Application.h>
#include <OS.h>
#include <Volume.h>

class BMessage;
class BString;
class BVolume;
class BDirectory;


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

	BDirectory*			fTrashDir;
	BString				fEventDir;
	BVolume				fQueryVolume;
};

#endif
