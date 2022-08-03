/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CALENDAR_DAEMON_H
#define CALENDAR_DAEMON_H

#include <Application.h>

#include "Event.h"

enum
{
	kEventAdded,
	kEventRemoved,
	kEventChanged,
	kEventNotify,
};

class BMessage;
class BString;


class CalendarDaemon : public BApplication
{
public:
		CalendarDaemon();
		~CalendarDaemon();
	
	void		MessageReceived(BMessage* message);
	bool		QuitRequested();
	
	void		SayHello(const char* name);
	//void		Notify(Event* event);

private:

	BString		fEventDir;

};

int main();


#endif // _H
