/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORYEDITWINDOW_H
#define CATEGORYEDITWINDOW_H


#include <Window.h>


class BButton;
class BColorControl;
class BTextControl;
class BView;
class Category;
class ColorPreview;


const uint32 kCategoryEditQuitting = 'kceq';
const uint32 kRefreshCategoryList = 'krcl';
const uint32 kColorDropped ='kcld';
const uint32 kUpdateColor = 'kucl';


class CategoryEditWindow: public BWindow {
public:
				CategoryEditWindow();

	virtual void		MessageReceived(BMessage* message);
	virtual bool		QuitRequested();

	void			SetCategory(Category* category);

private:
	void			_InitInterface();
	void			_SetCurrentColor(rgb_color color);
	void			_OnOkPressed();
	void			_OnRevertPressed();
	void			_CategoryModified();
	void			_RefreshWindows();
	void			_CloseWindow();
	bool			_SaveChanges();

	static const uint32	kOkPressed				= 1000;
	static const uint32	kRevertPressed			= 1001;
	static const uint32	kCategoryTextChanged	= 1002;

	BView*			fMainView;
	BTextControl*	fCategoryText;
	BColorControl*	fPicker;
	ColorPreview*	fColorPreview;
	BButton*		fOkButton;
	BButton*		fRevertButton;

	BString			fOriginalText;
	rgb_color		fOriginalColor;

	Category*		fCategory;
};


#endif
