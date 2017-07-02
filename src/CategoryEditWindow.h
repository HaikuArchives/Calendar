/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORYEDITWINDOW_H
#define CATEGORYEDITWINDOW_H


#include <Button.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>


const uint32 kCategoryEditQuitting = 'kceq';


class CategoryEditWindow: public BWindow {
public:
			CategoryEditWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:

	static const uint32 kSavePressed = 1000;
	static const uint32 kDeletePressed = 1001;

	BView*				fMainView;
	BTextControl*		fCategoryText;
	BButton*			fSaveButton;
	BButton*			fDeleteButton;
};


#endif
