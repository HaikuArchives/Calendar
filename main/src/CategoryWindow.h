/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORYWINDOW_H
#define CATEGORYWINDOW_H

#include <Window.h>

#include "Category.h"

class BButton;
class BListView;
class BScrollView;
class BView;
class Category;
class CategoryEditWindow;
class QueryDBManager;


const uint32 kCategoryWindowQuitting = 'kcwq';


class CategoryWindow : public BWindow
{
public:
					CategoryWindow();
					~CategoryWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

	void			LoadCategories();
	QueryDBManager*	GetDBManager();

private:
	void			_InitInterface();
	void			_OpenCategoryWindow(Category* category);
	void			_OnDeletePressed();

	static const uint32 kAddPressed = 1000;
	static const uint32 kDeletePressed = 1001;
	static const uint32 kCategoryEditSelected = 1002;
	static const uint32 kCategorySelected = 1003;

	BView*			fMainView;
	BListView*		fCategoryListView;
	BScrollView*	fCategoryScroll;
	BButton*		fNewButton;
	BButton*		fDeleteButton;
	BButton*		fEditButton;
	CategoryEditWindow* fCategoryEditWindow;

	CategoryList*	fCategoryList;
	QueryDBManager*	fDBManager;
};


#endif
