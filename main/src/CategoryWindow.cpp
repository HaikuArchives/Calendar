/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <NodeInfo.h>
#include <ScrollView.h>
#include <View.h>

#include "App.h"
#include "CategoryEditWindow.h"
#include "CategoryListItem.h"
#include "Preferences.h"
#include "QueryDBManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CategoryWindow"


CategoryWindow::CategoryWindow()
	:
	BWindow(BRect(), B_TRANSLATE("Manage categories"), B_TITLED_WINDOW,
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
	switch (message->what) {

		case kAddPressed:
			_OpenCategoryWindow(NULL);
			break;

		case kDeletePressed:
			_OnDeletePressed();
			break;

		case kCategoryEditSelected:
		{
			int32 selection = fCategoryListView->CurrentSelection();
			if (selection >= 0) {
				Category* category = fCategoryList->ItemAt(selection);
				_OpenCategoryWindow(category);
			}
			break;
		}

		case kCategorySelected:
		{
			bool enabled = message->GetInt32("index", -1) >= 0;
			fEditButton->SetEnabled(enabled);
			fDeleteButton->SetEnabled(enabled);
			break;
		}

		case kCategoryEditQuitting:
		{
			fCategoryEditWindow = NULL;
			LoadCategories();
			break;
		}

		case B_REFS_RECEIVED:
		{
			int i = 0;
			entry_ref ref;
			BFile file;
			BNodeInfo info;
			char type[B_FILE_NAME_LENGTH];
			QueryDBManager DBManager(((App*) be_app)->GetPreferences()->fDefaultCategory);

			while (message->HasRef("refs", i)) {
				message->FindRef("refs", i++, &ref);

				file.SetTo(&ref, B_READ_ONLY);
				info.SetTo(&file);
				info.GetType(type);

				if (BString(type) == BString("application/x-calendar-category"))
					_OpenCategoryWindow(fDBManager->GetCategory(ref));
				else {
					BMessage msg = BMessage(B_REFS_RECEIVED);
					msg.AddRef("refs", &ref);
					((App*) be_app)->PostMessage(&msg);
				}
			}
			break;
		}

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

	int32 selection = fCategoryListView->CurrentSelection();

	delete fCategoryList;
	fCategoryList = fDBManager->GetAllCategories();

	Category* category;
	for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
		category = ((Category*) fCategoryList->ItemAt(i));
		fCategoryListView->AddItem(
			new CategoryListItem(category->GetName(), category->GetColor()));
	}

	if (selection < fCategoryList->CountItems())
		fCategoryListView->Select(selection);
	else
		MessageReceived(new BMessage(kCategorySelected));

	fCategoryListView->Invalidate();
	UnlockLooper();
}


QueryDBManager*
CategoryWindow::GetDBManager()
{
	return fDBManager;
}


void
CategoryWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fCategoryListView = new BListView(
		"CategoryListView", B_SINGLE_SELECTION_LIST, B_WILL_DRAW);

	fCategoryScroll = new BScrollView(
		"CategoryScroll", fCategoryListView, B_WILL_DRAW, false, true);
	fCategoryScroll->SetExplicitMinSize(BSize(260, 220));

	fDBManager = new QueryDBManager(((App*) be_app)->GetPreferences()->fDefaultCategory);
	fCategoryList = NULL;
	LoadCategories();

	fCategoryListView->SetSelectionMessage(new BMessage(kCategorySelected));
	fCategoryListView->SetInvocationMessage(
		new BMessage(kCategoryEditSelected));

	fNewButton = new BButton("NewButton", B_TRANSLATE("New" B_UTF8_ELLIPSIS),
		new BMessage(kAddPressed));
	fDeleteButton = new BButton(
		"DeleteButton", B_TRANSLATE("Delete"), new BMessage(kDeletePressed));
	fEditButton = new BButton("EditButton", B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		new BMessage(kCategoryEditSelected));
	fEditButton->SetEnabled(false);
	fDeleteButton->SetEnabled(false);

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.Add(fCategoryScroll)
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.Add(fDeleteButton)
		.AddGlue()
		.Add(fNewButton)
		.Add(fEditButton)
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
		fCategoryEditWindow->SetCategory(category);
		fCategoryEditWindow->Show();
	}

	fCategoryEditWindow->Activate();
}


void
CategoryWindow::_OnDeletePressed()
{
	int32 selection = fCategoryListView->CurrentSelection();
	if (selection < 0)
		return;

	Category* category = fCategoryList->ItemAt(selection);
	BString defaultCat = ((App*) be_app)->GetPreferences()->fDefaultCategory;

	if (category->GetName() == defaultCat) {
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("You cannot delete the default category."), NULL,
			B_TRANSLATE("OK"), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->Go();
		return;
	}

	BAlert* alert = new BAlert(B_TRANSLATE("Confirm delete"),
		B_TRANSLATE("Are you sure you want to delete the selected category?"),
		NULL, B_TRANSLATE("OK"), B_TRANSLATE("Cancel"), B_WIDTH_AS_USUAL,
		B_WARNING_ALERT);

	alert->SetShortcut(1, B_ESCAPE);
	int32 button_index = alert->Go();

	if (button_index == 0) {
		if (fDBManager->RemoveCategory(category) == false) {
			BAlert* alert = new BAlert(B_TRANSLATE("Error"),
				B_TRANSLATE("Cannot delete category. Can't delete a category "
							"used by events."),
				NULL, B_TRANSLATE("OK"), NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			return;
		} else
			LoadCategories();
	}
}
