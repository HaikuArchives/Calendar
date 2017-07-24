/*
 * Copyright 2017 Akshay Agarwal<agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EVENTLISTVIEW_H
#define EVENTLISTVIEW_H

#include <ListView.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>


static const uint32 kPopClosed	= 'kpop';
static const uint32 kEventModify	= 'kemd';
static const uint32 kEventDelete	= 'kedt';


class EventListView : public BListView {
public:
				EventListView();
				~EventListView();

	virtual void		Draw(BRect rect);
	virtual	void		FrameResized(float w, float h);
	virtual	void		MessageReceived(BMessage* message);
	void			MouseDown(BPoint position);
	void			MouseUp(BPoint position);

private:
	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
	bool			fPrimaryButton;
	int32			fCurrentItemIndex;
};

#endif // EVENTLISTVIEW_H
