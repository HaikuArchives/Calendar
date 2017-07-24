/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EVENTLISTITEM_H
#define EVENTLISTITEM_H


#include <Font.h>
#include <InterfaceDefs.h>
#include <ListItem.h>
#include <MenuItem.h>
#include <String.h>


class EventListItem: public BListItem {
public:
				EventListItem(BString name, BString timeText);
				~EventListItem();

	virtual void		DrawItem(BView*, BRect, bool);
	virtual	void		Update(BView*, const BFont*);

private:
	static const int 	fItemHeight	= 32;
	static const rgb_color 	fDefaultColor;

	BString			fName;
	BString			fTimeText;
};

#endif // EVENTLLISTITEM_H
