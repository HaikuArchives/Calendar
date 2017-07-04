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
#include <Screen.h>

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
	fTextStartDate = new BTextControl("StartDate", NULL, NULL, NULL);
	fTextEndDate = new BTextControl("EndDate", NULL, NULL, NULL);
	fTextStartTime = new BTextControl("StartTime", NULL, NULL, NULL);
	fTextEndTime = new BTextControl("EndTime", NULL, NULL, NULL);

	fTextDescription = new BTextView("TextDescription", B_WILL_DRAW);
	fTextDescription->MakeEditable();
	fTextDescription->SetExplicitMinSize(BSize(B_SIZE_UNSET, 100));

	fAllDayCheckBox = new BCheckBox("", new BMessage(kAllDayPressed));
	fAllDayCheckBox->SetValue(B_CONTROL_OFF);
	fStartTimeCheckBox = new BCheckBox("AM", B_OK);
	fEndTimeCheckBox = new BCheckBox("PM", B_OK);

	fEveryMonth = new BRadioButton("EveryMonth", "Monthly", new BMessage(kOptEveryMonth));
	fEveryYear = new BRadioButton("EveryYear", "Yearly", new BMessage(kOptEveryYear));

	fNameLabel = new BStringView("NameLabel", "Name:");
	fPlaceLabel = new BStringView("PlaceLabel", "Place:");
	fDescriptionLabel = new BStringView("DescriptionLabel", "Description:");
	fCategoryLabel = new BStringView("CategoryLabel", "Category:");
	fAllDayLabel = new BStringView("AllDayLabel", "All Day:");
	fEndDateLabel = new BStringView("EndDateLabel", "End Date:");
	fStartDateLabel = new BStringView("StartDateLabel", "Start Date:");
	fRecurrenceLabel = new BStringView("RecurrenceLabel", "Recurrence:");
	fStartTimeLabel = new BStringView("StartTimeLabel", "Start Time:");
	fEndTimeLabel = new BStringView("EndTimeLabel", "End Time:");

	fDeleteButton = new BButton("DeleteButton", "Delete", new BMessage(kDeletePressed));
	fStartCalButton = new BButton("StartCalButton", "▼", new BMessage(kShowStartDateCalendar));
	fEndCalButton = new BButton("EndCalButton", "▼", new BMessage(kShowEndDateCalendar));
	BButton* CancelButton = new BButton("CancelButton", "Cancel", new BMessage(kCancelPressed));
	BButton* SaveButton = new BButton("SaveButton", "Save", new BMessage(kSavePressed));

	float width, height;
	fStartDateLabel->GetPreferredSize(&width, &height);
	fStartCalButton->SetExplicitMinSize(BSize(height, height));
	fEndCalButton->SetExplicitMinSize(BSize(height, height));

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

	BBox* fRecurrenceBox = new BBox("RecurrenceBox");

	BLayoutBuilder::Group<>(fRecurrenceBox, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.Add(fRecurrenceLabel)
		.Add(fEveryMonth)
		.Add(fEveryYear)
	.End();

	fTextStartDate->SetEnabled(false);
	fTextEndDate->SetEnabled(false);
	fTextStartTime->SetEnabled(false);
	fTextEndTime->SetEnabled(false);

	fStartDateBox = new BBox("Start Date and Time");
	BLayoutBuilder::Group<>(fStartDateBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.SetInsets(B_USE_ITEM_INSETS)
			.AddStrut(B_USE_ITEM_SPACING)
			.AddGrid()
				.Add(fStartDateLabel, 0, 0)
				.Add(fTextStartDate, 1, 0)
				.Add(fStartCalButton, 2, 0)
				.Add(fStartTimeLabel, 0, 1)
				.Add(fTextStartTime, 1, 1)
				.Add(fStartTimeCheckBox, 2, 1)
			.End()
	.End();
	fStartDateBox->SetLabel("Start Date and Time");

	fEndDateBox = new BBox("Start Date and Time");
	BLayoutBuilder::Group<>(fEndDateBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.SetInsets(B_USE_ITEM_INSETS)
			.AddStrut(B_USE_ITEM_SPACING)
			.AddGrid()
				.Add(fEndDateLabel, 0, 0)
				.Add(fTextEndDate, 1, 0)
				.Add(fEndCalButton, 2, 0)
				.Add(fEndTimeLabel, 0, 1)
				.Add(fTextEndTime, 1, 1)
				.Add(fEndTimeCheckBox, 2, 1)
			.End()
	.End();
	fEndDateBox->SetLabel("End Date and Time");


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
		.End()
		.Add(fStartDateBox)
		.Add(fEndDateBox)
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

		case kShowStartDateCalendar:
		{
			BPoint where;
			BPoint boxPosition = fStartDateBox->Frame().LeftTop();
			BPoint buttonPosition = fStartCalButton->Frame().LeftBottom();
			where = boxPosition + buttonPosition;
			where += BPoint(10.0, 8.0);
			ShowCalendar(where);
			break;
		}

		case kShowEndDateCalendar:
		{
			BPoint where;
			BPoint boxPosition = fEndDateBox->Frame().LeftTop();
			BPoint buttonPosition = fEndCalButton->Frame().LeftBottom();
			where = boxPosition + buttonPosition;
			where += BPoint(10.0, 8.0);
			ShowCalendar(where);
			break;
		}

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


void
EventWindow::ShowCalendar(BPoint where)
{
	if (fCalendarWindow.IsValid()) {
		BMessage activate(B_SET_PROPERTY);
		activate.AddSpecifier("Active");
		activate.AddBool("data", true);

		if (fCalendarWindow.SendMessage(&activate) == B_OK)
			return;
	}

	ConvertToScreen(&where);

	CalendarMenuWindow* window = new CalendarMenuWindow(where);
	fCalendarWindow = BMessenger(window);

	window->Show();
}
