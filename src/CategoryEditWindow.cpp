/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryEditWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Catalog.h>
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
#include "MainWindow.h"
#include "Preferences.h"
#include "QueryDBManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CategoryEditWindow"

CategoryEditWindow::CategoryEditWindow()
	:
	BWindow(BRect(), B_TRANSLATE("Category Edit"), B_TITLED_WINDOW,
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
		B_TRANSLATE("New category"), new BMessage(kCategoryTextChanged));

	fSaveButton = new BButton("SaveButton", B_TRANSLATE("Save"),
		new BMessage(kSavePressed));
	fDeleteButton = new BButton("DeleteButton", B_TRANSLATE("Delete"),
		new BMessage(kDeletePressed));


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
	BString defaultCat = ((App*)be_app)->GetPreferences()->fDefaultCategory;

	if (BString(fCategoryText->Text()) == defaultCat) {
		BAlert* alert  = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("You cannot delete the default category."),
			NULL, B_TRANSLATE("OK"),NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->Go();
		return;
	}

	BAlert* alert = new BAlert(B_TRANSLATE("Confirm delete"),
		B_TRANSLATE("Are you sure you want to delete the selected category?"),
		NULL, B_TRANSLATE("OK"), B_TRANSLATE("Cancel"),
		B_WIDTH_AS_USUAL, B_WARNING_ALERT);

	alert->SetShortcut(1, B_ESCAPE);
	int32 button_index = alert->Go();

	if (button_index == 0) {

		CategoryWindow* parent = ((App*)be_app)->categoryWindow();
		if (parent->GetDBManager()->RemoveCategory(fCategory)) {
			parent->LoadCategories();
			_RefreshWindows();
			_CloseWindow();
		}
		else
		{
			BAlert* alert  = new BAlert(B_TRANSLATE("Error"),
				B_TRANSLATE("Cannot delete category. Can't delete a category used by events."),
				NULL, B_TRANSLATE("OK"),NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
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
CategoryEditWindow::_RefreshWindows()
{
	((App*)be_app)->mainWindow()->PostMessage(kRefreshCategoryList);
}


void
CategoryEditWindow::_OnSavePressed()
{
	if (BString(fCategoryText->Text()).CountChars() < 3) {

		BAlert* alert  = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("The name must have a length greater than 2."),
			NULL, B_TRANSLATE("OK"),NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->Go();
		return;
	}

	Category category(fCategoryText->Text(), fPicker->ValueAsColor());
	CategoryWindow* parent = ((App*)be_app)->categoryWindow();

	if ((fCategory == NULL) && (parent->GetDBManager()->AddCategory(&category))) {
		parent->LoadCategories();
		_RefreshWindows();
		_CloseWindow();
	}

	else if ((fCategory != NULL) && (parent->GetDBManager()->UpdateCategory(fCategory, &category)))
	{
		parent->LoadCategories();
		_RefreshWindows();
		_CloseWindow();
	}

	else
	{
		BAlert* alert  = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("Cannot add/modify the category. A category with the same name or color already exists."),
			NULL, B_TRANSLATE("OK"),NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return;

	}

}
