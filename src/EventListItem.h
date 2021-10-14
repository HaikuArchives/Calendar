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


class EventListItem: public BListItem {
public:
				EventListItem(BString name, BString timeText,
					rgb_color color, uint16 font_face = -1);
				~EventListItem();

	virtual void		DrawItem(BView*, BRect, bool);
	virtual	void		Update(BView*, const BFont*);

private:
	static const int 	fItemHeight	= 42;

	BString			fName;
	BString			fTimeText;
	rgb_color		fColor;
	uint16			fFace;
};

#endif // EVENTLLISTITEM_H
