/*
 * Copyright 2017 Akshay Agarwal<agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _MAINVIEW_H
#define _MAINVIEW_H


#include <DateTime.h>
#include <Message.h>
#include <View.h>


const uint32 kSystemDateChangeMessage = 'ksdm';
const uint32 kDateChange = 'kdcm';


class MainView : public BView {
public:
								MainView();

	virtual	void			 	Pulse();
	virtual	void				AttachedToWindow();

private:
			void				_SendNotices();
			BMessage			fMessage;
			BDate				fCurrentDate;
};


#endif // _H
