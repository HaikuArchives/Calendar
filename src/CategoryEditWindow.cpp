/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryEditWindow.h"

#include <Application.h>
#include <LayoutBuilder.h>

#include "App.h"

CategoryEditWindow::CategoryEditWindow()
	:
	BWindow(BRect(), "Category Edit", B_TITLED_WINDOW,
			B_AUTO_UPDATE_SIZE_LIMITS)
{
	CenterOnScreen();

	fMainView = new BView("MainView", B_WILL_DRAW);
	fCategoryText = new BTextControl("CategoryText", NULL, NULL, NULL);
	fSaveButton = new BButton("SaveButton", "Save", new BMessage(kSavePressed));
	fDeleteButton = new BButton("DeleteButton", "Delete", new BMessage(kDeletePressed));

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL)
		.SetInsets(B_USE_SMALL_INSETS)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fCategoryText)
			.AddGlue()
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fSaveButton)
			.Add(fDeleteButton)
		.End()
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMainView)
	.End();
}


void
CategoryEditWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
CategoryEditWindow::QuitRequested()
{
	((App*)be_app)->categoryWindow()->PostMessage(kCategoryEditQuitting);
	return true;
}
