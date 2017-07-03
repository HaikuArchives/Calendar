/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORYEDITWINDOW_H
#define CATEGORYEDITWINDOW_H


#include <Button.h>
#include <ColorControl.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

#include "ColorPreview.h"


const uint32 kCategoryEditQuitting = 'kceq';
const uint32 kColorDropped ='kcld';
const uint32 kUpdateColor = 'kucl';


class CategoryEditWindow: public BWindow {
public:
			CategoryEditWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:

	void				_SetCurrentColor(rgb_color color);

	static const uint32 kSavePressed = 1000;
	static const uint32 kDeletePressed = 1001;

	BView*			fMainView;
	BTextControl*		fCategoryText;
	BColorControl*		fPicker;
	ColorPreview*		fColorPreview;
	BButton*		fSaveButton;
	BButton*		fDeleteButton;


};


#endif
