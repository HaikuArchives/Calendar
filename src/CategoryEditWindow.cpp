/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryEditWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <ColorControl.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <SeparatorView.h>
#include <TextControl.h>
#include <View.h>

#include "App.h"
#include "Application.h"
#include "Category.h"
#include "CategoryWindow.h"
#include "ColorPreview.h"
#include "SQLiteManager.h"


CategoryEditWindow::CategoryEditWindow()
	:
	BWindow(BRect(), "Category Edit", B_TITLED_WINDOW,
			B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	_InitInterface();
	CenterOnScreen();
}


void
CategoryEditWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kUpdateColor:
		{
			rgb_color color = fPicker->ValueAsColor();
			_SetCurrentColor(color);
			_CategoryModified();
			break;
		}

		case kSavePressed:
			_OnSavePressed();
			break;

		case kDeletePressed:
			_OnDeletePressed();
			break;

		case kCategoryTextChanged:
			_CategoryModified();
			break;

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


void
CategoryEditWindow::SetCategory(Category* category)
{
	fCategory = category;

	if (fCategory != NULL) {
		fCategoryText->SetText(category->GetName());
		fPicker->SetValue(category->GetColor());
		fColorPreview->SetColor(category->GetColor());
		fColorPreview->Invalidate();

		fSaveButton->SetEnabled(false);
	}

	else
	{
		fPicker->SetValue((rgb_color){255, 255, 0});
		fColorPreview->SetColor((rgb_color){255, 255, 0});
		fColorPreview->Invalidate();
		fDeleteButton->SetEnabled(false);

	}

	fCategoryText->SetModificationMessage(new BMessage(kCategoryTextChanged));
}


void
CategoryEditWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fCategoryText = new BTextControl("CategoryText", NULL,
		"New Category", new BMessage(kCategoryTextChanged));

	fSaveButton = new BButton("SaveButton", "Save", new BMessage(kSavePressed));
	fDeleteButton = new BButton("DeleteButton", "Delete", new BMessage(kDeletePressed));


	BRect wellrect(0, 0, 49, 49);
	fColorPreview = new ColorPreview(wellrect, new BMessage(kColorDropped), 0);
	fColorPreview->SetExplicitAlignment(BAlignment(B_ALIGN_HORIZONTAL_CENTER,
		B_ALIGN_BOTTOM));

	fPicker = new BColorControl(B_ORIGIN, B_CELLS_16x16, 1,
		"picker", new BMessage(kUpdateColor));

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fCategoryText)
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fColorPreview)
			.Add(fPicker)
		.End()
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMainView)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.SetInsets(B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING,
				B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(fSaveButton)
			.Add(fDeleteButton)
		.End()
	.End();

	fColorPreview->Parent()->SetExplicitMaxSize(
		BSize(B_SIZE_UNSET, fCategoryText->Bounds().Height()));
}


void
CategoryEditWindow::_SetCurrentColor(rgb_color color)
{
	fPicker->SetValue(color);
	fColorPreview->SetColor(color);
	fColorPreview->Invalidate();
}


void
CategoryEditWindow::_CategoryModified()
{
	fSaveButton->SetEnabled(true);
}


void
CategoryEditWindow::_OnDeletePressed()
{
	BAlert* alert = new BAlert("Confirm delete",
		"Are you sure you want to delete the selected category?",
		NULL, "OK", "Cancel", B_WIDTH_AS_USUAL, B_WARNING_ALERT);

	alert->SetShortcut(1, B_ESCAPE);
	int32 button_index = alert->Go();

	if (button_index == 0) {

		CategoryWindow* parent = ((App*)be_app)->categoryWindow();
		if (parent->GetDBManager()->RemoveCategory(fCategory)) {
			parent->LoadCategories();
			_CloseWindow();
		}
		else
		{
			BAlert* alert  = new BAlert("Error",
				"Cannot Delete category. Can't delete a category used by events.",
				NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			return;
		}

	}
}


void
CategoryEditWindow::_CloseWindow()
{
	PostMessage(B_QUIT_REQUESTED);
}


void
CategoryEditWindow::_OnSavePressed()
{
	if (BString(fCategoryText->Text()).CountChars() < 3) {

		BAlert* alert  = new BAlert("Error",
			"The name must have a length greater than 2.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->Go();
		return;
	}

	Category category(0, fCategoryText->Text(), fPicker->ValueAsColor());
	CategoryWindow* parent = ((App*)be_app)->categoryWindow();

	if ((fCategory == NULL) && (parent->GetDBManager()->AddCategory(&category))) {
		parent->LoadCategories();
		_CloseWindow();
	}

	else if ((fCategory != NULL) && (parent->GetDBManager()->UpdateCategory(fCategory, &category)))
	{
		parent->LoadCategories();
		_CloseWindow();
	}

	else
	{
		BAlert* alert  = new BAlert("Error",
			"Cannot Add/Modify category. Try with a different name and color.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return;

	}

}
