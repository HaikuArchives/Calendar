/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EVENTLISTVIEW_H
#define EVENTLISTVIEW_H

#include <ListView.h>


static const uint32 kPopClosed	= 'kpop';


class EventListView : public BListView {
public:
				EventListView();
				~EventListView();

	virtual void		Draw(BRect rect);
	virtual	void		FrameResized(float w, float h);
	virtual	void		MessageReceived(BMessage* message);
	void			MouseDown(BPoint position);
	void			MouseUp(BPoint position);

	void			SetPopUpMenuEnabled(bool enable);

private:
	static const int32 kEditActionInvoked	= 1000;
	static const int32 kDeleteActionInvoked	= 1001;
	static const int32 kCancelActionInvoked	= 1002;

	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
	bool			fPopUpMenuEnabled;
	bool			fPrimaryButton;
	int32			fCurrentItemIndex;
};

#endif // EVENTLISTVIEW_H
