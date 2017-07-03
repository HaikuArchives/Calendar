/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "CategoryEditWindow.h"

#include <Application.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>

#include "App.h"

CategoryEditWindow::CategoryEditWindow()
	:
	BWindow(BRect(), "Category Edit", B_TITLED_WINDOW,
			B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	CenterOnScreen();

	fMainView = new BView("MainView", B_WILL_DRAW);
	fCategoryText = new BTextControl("CategoryText", NULL,
		"New Category", NULL);
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
CategoryEditWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		if (message->WasDropped()) {
		rgb_color* color = NULL;
		ssize_t size = 0;

		if (message->FindData("RGBColor", (type_code)'RGBC', (const void**)&color,
				&size) == B_OK) {
			_SetCurrentColor(*color);
		}
	}

		case kUpdateColor:
		{
			rgb_color color = fPicker->ValueAsColor();
			_SetCurrentColor(color);
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
CategoryEditWindow::_SetCurrentColor(rgb_color color)
{
	fPicker->SetValue(color);
	fColorPreview->SetColor(color);
	fColorPreview->Invalidate();
}


bool
CategoryEditWindow::QuitRequested()
{
	((App*)be_app)->categoryWindow()->PostMessage(kCategoryEditQuitting);
	return true;
}
