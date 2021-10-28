/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EVENTLISTITEM_H
#define EVENTLISTITEM_H


#include <InterfaceDefs.h>
#include <ListItem.h>
#include <String.h>

class Event;


class EventListItem: public BListItem {
public:
				EventListItem(Event* event, int32 mode);
				~EventListItem();

	virtual void		DrawItem(BView*, BRect, bool);
	virtual	void		Update(BView*, const BFont*);

			Event*		GetEvent();

private:
			void		_CalcTimeText(int32 mode);

	static const int 	fItemHeight	= 42;

	Event*			fEvent;
	BString			fTimeText;
};

#endif // EVENTLLISTITEM_H
