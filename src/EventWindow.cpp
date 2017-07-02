/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EventWindow.h"

#include <Application.h>
#include <Box.h>
#include <GroupLayout.h>
#include <LayoutItem.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>

#include "App.h"
#include "MainWindow.h"

EventWindow::EventWindow()
	:
	BWindow(BRect(), "Event Manager", B_TITLED_WINDOW,
			B_AUTO_UPDATE_SIZE_LIMITS)
{
	CenterOnScreen();

	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fTextName = new BTextControl("EventName", NULL, NULL, NULL);
	fTextPlace = new BTextControl("EventPlace", NULL, NULL, NULL);

	fTextDescription = new BTextView("TextDescription", B_WILL_DRAW);
	fTextDescription->MakeEditable();

	fDeleteButton = new BButton("DeleteButton", "Delete", new BMessage(kDeletePressed));
	BButton* CancelButton = new BButton("CancelButton", "Cancel", new BMessage(kCancelPressed));
	BButton* SaveButton = new BButton("SaveButton", "Save", new BMessage(kSavePressed));

	fAllDayCheckBox = new BCheckBox("", new BMessage(kAllDayPressed));
	fAllDayCheckBox->SetValue(B_CONTROL_OFF);

	fEveryMonth = new BRadioButton("EveryMonth", "Monthly", new BMessage(kOptEveryMonth));
	fEveryYear = new BRadioButton("EveryYear", "Yearly", new BMessage(kOptEveryYear));

	fNameLabel = new BStringView("NameLabel", "Name");
	fPlaceLabel = new BStringView("PlaceLabel", "Place");
	fDescriptionLabel = new BStringView("DescriptionLabel", "Description");
	fCategoryLabel = new BStringView("CategoryLabel", "Category");
	fAllDayLabel = new BStringView("AllDayLabel", "All Day");
	fStartLabel = new BStringView("StartLabel", "Start");
	fEndLabel = new BStringView("EndLabel", "End");
	fRecurrenceLabel = new BStringView("RecurrenceLabel", "Recurrence");

	fCategoryMenu = new BMenu("CategoryMenu");
	fCategoryMenu->AddItem(new BMenuItem("Default", B_OK));
	fCategoryMenu->AddItem(new BMenuItem("Category", B_OK));

	fCategoryMenu->SetRadioMode(true);
	fCategoryMenu->SetLabelFromMarked(true);
	fCategoryMenu->SetRadioMode(true);
	fCategoryMenu->ItemAt(0)->SetMarked(true);

	fStartDateEdit = new BMenu("Start Date");
	fEndDateEdit = new BMenu("End Date");

	fCategoryMenuField = new BMenuField("CategoryMenuField", NULL, fCategoryMenu);
	fStartDateField = new BMenuField("StarDateField", NULL, fStartDateEdit);
	fEndDateField = new BMenuField("EndDateField", NULL,  fEndDateEdit);

	BBox* fRecurrenceBox = new BBox("RecurrenceBox");

	BLayoutBuilder::Group<>(fRecurrenceBox, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.Add(fRecurrenceLabel)
		.Add(fEveryMonth)
		.Add(fEveryYear)
	.End();

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.AddGrid()
			.Add(fNameLabel, 0, 0)
			.Add(fTextName, 1, 0)
			.Add(fPlaceLabel, 0, 1)
			.Add(fTextPlace, 1 ,1)
		.End()
		.AddGroup(B_VERTICAL)
			.Add(fDescriptionLabel)
			.Add(fTextDescription)
		.End()
		.AddGrid()
			.Add(fCategoryLabel, 0, 0)
			.Add(fCategoryMenuField, 1, 0)
			.Add(fAllDayLabel, 0, 1)
			.Add(fAllDayCheckBox, 1 ,1)
			.Add(fStartLabel, 0, 2)
			.Add(fStartDateField, 1 ,2)
			.Add(fEndLabel, 0, 3)
			.Add(fEndDateField, 1 ,3)
		.End()
		.AddStrut(B_USE_ITEM_SPACING)
		.Add(fRecurrenceBox)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(CancelButton)
			.Add(fDeleteButton)
			.Add(SaveButton)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_SMALL_INSETS)
		.SetInsets(B_USE_SMALL_INSETS)
		.Add(fMainView)
	.End();

}


void
EventWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
EventWindow::QuitRequested()
{
	be_app->PostMessage(kEventWindowQuitting);
	return true;
}
