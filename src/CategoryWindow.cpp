/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryWindow.h"


#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <View.h>

#include "Category.h"
#include "CategoryEditWindow.h"
#include "CategoryListItem.h"
#include "QueryDBManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CategoryWindow"

CategoryWindow::CategoryWindow()
	:
	BWindow(BRect(), B_TRANSLATE("Category manager"),
		B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fCategoryEditWindow(NULL)
{
	_InitInterface();
	CenterOnScreen();
}


CategoryWindow::~CategoryWindow()
{
	delete fCategoryList;
	delete fDBManager;
}


void
CategoryWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kAddPressed:
			_OpenCategoryWindow(NULL);
			break;

		case kCancelPressed:
			PostMessage(B_QUIT_REQUESTED);
			break;

		case kCategorySelected:
		{

			int32 selection = fCategoryListView->CurrentSelection();
			if (selection >= 0) {
				Category* category = ((Category*)fCategoryList->ItemAt(selection));
				_OpenCategoryWindow(category);
			}
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


void
CategoryWindow::LoadCategories()
{
	LockLooper();

	if(!fCategoryList->IsEmpty()) {
		fCategoryListView->MakeEmpty();
		fCategoryList->MakeEmpty();
	}

	fCategoryList = fDBManager->GetAllCategories();

	Category* category;
	for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
		category = ((Category*)fCategoryList->ItemAt(i));
		fCategoryListView->AddItem(new CategoryListItem(category->GetName(),
			category->GetColor()));
	}

	fCategoryListView->Invalidate();
	UnlockLooper();
}


QueryDBManager*
CategoryWindow::GetDBManager()
{
	return fDBManager;
}


void
CategoryWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


void
CategoryWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fCategoryListView = new BListView("CategoryListView", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW);

	fCategoryScroll = new BScrollView("CategoryScroll", fCategoryListView,
		B_WILL_DRAW, false, true);
	fCategoryScroll->SetExplicitMinSize(BSize(260, 220));

	fDBManager = new QueryDBManager();
	fCategoryList = new BList();
	LoadCategories();

	fCategoryListView->SetInvocationMessage(new BMessage(kCategorySelected));

	fAddButton = new BButton("AddButton", B_TRANSLATE("Add"),
		new BMessage(kAddPressed));
	fCancelButton = new BButton("CancelButton", B_TRANSLATE("Cancel"),
		new BMessage(kCancelPressed));

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
CategoryWindow::_OpenCategoryWindow(Category* category)
{
	if (fCategoryEditWindow == NULL) {
		fCategoryEditWindow = new CategoryEditWindow();
		fCategoryEditWindow->SetPreferences(fPreferences);
		fCategoryEditWindow->SetCategory(category);
		fCategoryEditWindow->Show();
	}

	fCategoryEditWindow->Activate();
}
