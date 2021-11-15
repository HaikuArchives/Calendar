/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EVENTLISTVIEW_H
#define EVENTLISTVIEW_H

#include <ListView.h>

class Event;

static const uint32 kPopClosed	= 'kpop';


class EventListView : public BListView {
public:
					EventListView(const char* name);
					~EventListView();

	virtual void	Draw(BRect rect);
	virtual	void	FrameResized(float w, float h);
	virtual	void	MessageReceived(BMessage* message);

	virtual void	MouseDown(BPoint position);
	virtual void	MouseUp(BPoint position);
	virtual void	SelectionChanged();
	virtual void	MakeEmpty();

			Event*	SelectedEvent();

	void			SetPopUpMenuEnabled(bool enable);

private:
	static const int32 kEditActionInvoked	= 1000;
	static const int32 kDeleteActionInvoked	= 1001;
	static const int32 kCancelActionInvoked	= 1002;
	static const int32 kHideActionInvoked	= 1003;

	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
	bool			fPopUpMenuEnabled;
	bool			fPrimaryButton;
	int32			fCurrentItemIndex;
};

#endif // EVENTLISTVIEW_H
