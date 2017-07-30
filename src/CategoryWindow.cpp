/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryWindow.h"


#include <Application.h>
#include <LayoutBuilder.h>

#include "CategoryListItem.h"


CategoryWindow::CategoryWindow()
	:
	BWindow(BRect(), "Category Manager",B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fCategoryEditWindow(NULL)
{

	CenterOnScreen();

	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fCategoryListView = new BListView("CategoryListView", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW);

	fCategoryScroll = new BScrollView("CategoryScroll", fCategoryListView,
		B_WILL_DRAW, false, true);
	fCategoryScroll->SetExplicitMinSize(BSize(260, 260));

	fCategoryListView->SetInvocationMessage(new BMessage(kCategorySelected));
	fCategoryListView->AddItem(new CategoryListItem("Default", (rgb_color){86, 86, 197}));
	fCategoryListView->AddItem(new CategoryListItem("Birthdays", (rgb_color){194, 86, 86}));

	fAddButton = new BButton("AddButton", "Add", new BMessage(kAddPressed));
	fCancelButton = new BButton("CancelButton", "Cancel", new BMessage(kCancelPressed));

	fAddButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
	fCancelButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL)
		.Add(fCategoryScroll)
		.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.Add(fAddButton)
			.Add(fCancelButton)
		.End()
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fMainView)
	.End();
}


void
CategoryWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kAddPressed:
		{
			if (fCategoryEditWindow == NULL) {
				fCategoryEditWindow = new CategoryEditWindow();
				fCategoryEditWindow->Show();
			}

			fCategoryEditWindow->Activate();
			break;
		}

		case kCategoryEditQuitting:
			fCategoryEditWindow = NULL;
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
CategoryWindow::QuitRequested()
{
	if (fCategoryEditWindow != NULL) {
		fCategoryEditWindow->Activate();
		return false;
	}

	be_app->PostMessage(kCategoryWindowQuitting);
	return true;
}
