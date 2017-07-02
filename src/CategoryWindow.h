/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORYWINDOW_H
#define CATEGORYWINDOW_H


#include <Button.h>
#include <ListView.h>
#include <ScrollView.h>
#include <View.h>
#include <Window.h>


const uint32 kCategoryWindowQuitting = 'kcwq';


class CategoryWindow: public BWindow {
public:
			CategoryWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	static const uint32 kAddPressed		= 1000;
	static const uint32 kCancelPressed	= 1001;
	static const uint32 kCategorySelected	= 1002;

	BView*			fMainView;
	BListView*		fCategoryList;
	BScrollView*		fCategoryScroll;
	BButton*		fAddButton;
	BButton*		fCancelButton;

};


#endif
