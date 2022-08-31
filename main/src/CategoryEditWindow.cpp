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
	BWindow(BRect(), B_TRANSLATE("Edit category"), B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	_InitInterface();
	CenterOnScreen();
}


void
CategoryEditWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {

		case kUpdateColor:
		{
			rgb_color color = fPicker->ValueAsColor();
			_SetCurrentColor(color);
			_CategoryModified();
			break;
		}

		case kCategoryTextChanged:
			_CategoryModified();
			break;

		case kOkPressed:
			_OnOkPressed();
			break;

		case kRevertPressed:
			_OnRevertPressed();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
CategoryEditWindow::QuitRequested()
{
	_RefreshWindows();
	((App*) be_app)->categoryWindow()->PostMessage(kCategoryEditQuitting);
	return true;
}


void
CategoryEditWindow::SetCategory(Category* category)
{
	fCategory = category;

	if (fCategory != NULL) {
		fOriginalText = category->GetName();
		fOriginalColor = category->GetColor();

		fCategoryText->SetText(fOriginalText);
		fPicker->SetValue(fOriginalColor);
		fColorPreview->SetColor(fOriginalColor);
		fColorPreview->Invalidate();
	}

	else {
		fPicker->SetValue((rgb_color){255, 255, 0});
		fColorPreview->SetColor((rgb_color){255, 255, 0});
		fColorPreview->Invalidate();
	}

	fRevertButton->SetEnabled(false);
	fCategoryText->SetModificationMessage(new BMessage(kCategoryTextChanged));
}


void
CategoryEditWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fCategoryText = new BTextControl("CategoryText", NULL,
		B_TRANSLATE("New category"), new BMessage(kCategoryTextChanged));

	fOkButton = new BButton("OkButton", B_TRANSLATE("OK"), new BMessage(kOkPressed));
	fRevertButton = new BButton(
		"RevertButton", B_TRANSLATE("Revert"), new BMessage(kRevertPressed));

	BRect wellrect(0, 0, 49, 49);
	fColorPreview = new ColorPreview(wellrect, new BMessage(kColorDropped), 0);
	fColorPreview->SetExplicitAlignment(
		BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_BOTTOM));

	fPicker = new BColorControl(
		B_ORIGIN, B_CELLS_16x16, 1, "picker", new BMessage(kUpdateColor));

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
		.Add(fRevertButton)
		.AddGlue()
		.Add(fOkButton)
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
	if (fOriginalText == NULL)
		return;
	fRevertButton->SetEnabled(true);
}


void
CategoryEditWindow::_OnRevertPressed()
{
	fCategoryText->SetText(fOriginalText);
	fPicker->SetValue(fOriginalColor);
	fColorPreview->SetColor(fOriginalColor);
	fColorPreview->Invalidate();
}


void
CategoryEditWindow::_CloseWindow()
{
	_RefreshWindows();
	PostMessage(B_QUIT_REQUESTED);
}


void
CategoryEditWindow::_RefreshWindows()
{
	((App*) be_app)->mainWindow()->PostMessage(kRefreshCategoryList);
}


void
CategoryEditWindow::_OnOkPressed()
{
	if (BString(fCategoryText->Text()).CountChars() < 3) {
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("The name must have a length greater than 2."), NULL,
			B_TRANSLATE("OK"), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->Go();
		return;
	}

	if (_SaveChanges() == false) {
		BAlert* alert = new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("Cannot add/modify the category. A category with the "
						"same name or color already exists."),
			NULL, B_TRANSLATE("OK"), NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return;
	}
	_CloseWindow();
}


bool
CategoryEditWindow::_SaveChanges()
{
	if (BString(fCategoryText->Text()).CountChars() > 3) {
		Category category(fCategoryText->Text(), fPicker->ValueAsColor());
		CategoryWindow* parent = ((App*) be_app)->categoryWindow();

		if (fCategory == NULL)
			return parent->GetDBManager()->AddCategory(&category);
		else if (fCategory != NULL)
			return parent->GetDBManager()->UpdateCategory(fCategory, &category);
	}
	return false;
}
