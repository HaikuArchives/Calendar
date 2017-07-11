/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EventWindow.h"

#include <Application.h>
#include <Alert.h>
#include <Box.h>
#include <DateFormat.h>
#include <File.h>
#include <GroupLayout.h>
#include <LayoutItem.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <Screen.h>
#include <TimeFormat.h>

#include "App.h"
#include "Event.h"
#include "MainWindow.h"


EventWindow::EventWindow()
	:
	BWindow(BRect(), "Event Manager", B_TITLED_WINDOW,
			B_AUTO_UPDATE_SIZE_LIMITS),
	fStartDateTime(),
	fEndDateTime()
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
	fEndTimeCheckBox = new BCheckBox("AM", B_OK);

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
	BButton* CancelButton = new BButton("CancelButton", "Cancel", new BMessage(kCancelPressed));
	BButton* SaveButton = new BButton("SaveButton", "Save", new BMessage(kSavePressed));

	BMessage* message = new BMessage(kShowPopUpCalendar);
	message->AddInt8("which",0);
	fStartCalButton = new BButton("StartCalButton", "▼", message);
	message = new BMessage(kShowPopUpCalendar);
	message->AddInt8("which",1);
	fEndCalButton = new BButton("EndCalButton", "▼", message);


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

	DisableControls();
}


void
EventWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kAllDayPressed:
			OnCheckBoxToggle();
			break;

		case kCancelPressed:
			PostMessage(B_QUIT_REQUESTED);
			break;

		case kDeletePressed:
			break;

		case kSavePressed:
			OnSaveClick();
			break;

		case kShowPopUpCalendar:
		{
			int8 which;
			message->FindInt8("which", &which);
			switch(which) {
				case 0:
				{
					BPoint where;
					BPoint boxPosition = fStartDateBox->Frame().LeftTop();
					BPoint buttonPosition = fStartCalButton->Frame().LeftBottom();
					where = boxPosition + buttonPosition;
					where += BPoint(10.0, 8.0);
					ShowCalendar(where);
					break;
				}

				case 1:
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
					break;
			}
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
EventWindow::SetEvent(Event* event, int eventIndex,
		BList* eventList)
{
	fEvent = event;
	fEventList = eventList;
	fEventIndex = eventIndex;

	if (event != NULL) {
		fTextName->SetText(event->GetName());
		fTextPlace->SetText(event->GetPlace());
		fTextDescription->SetText(event->GetDescription());

		fStartDateTime = event->GetStartDateTime();
		fEndDateTime = event->GetEndDateTime();

		fTextStartDate->SetText(GetLocaleDateString(fStartDateTime.Time_t()));
		fTextEndDate->SetText(GetLocaleDateString(fEndDateTime.Time_t()));

		if (event->IsAllDay()) {
			fAllDayCheckBox->SetValue(B_CONTROL_ON);
			fTextStartTime->SetText("");
			fTextEndTime->SetText("");
		}

		else
		{
			fAllDayCheckBox->SetValue(B_CONTROL_OFF);
			fTextEndTime->SetText(GetLocaleTimeString(fEndDateTime.Time_t()));
			fTextStartTime->SetText(GetLocaleTimeString(fStartDateTime.Time_t()));
		}

		fDeleteButton->SetEnabled(true);

	}

}


void
EventWindow::SetEventDate(BDate& date)
{
	BTime time;
	// Use a dummy start time for now(6:10 A.M)
	time.SetTime(6, 10, 0);
	fStartDateTime.SetTime(time);
	fStartDateTime.SetDate(date);

	// Use a dummy end time for now(7:10 A.M)
	time.AddHours(1);
	fEndDateTime.SetTime(time);
	fEndDateTime.SetDate(date);

	fTextStartDate->SetText(GetLocaleDateString(fStartDateTime.Time_t()));
	fTextStartTime->SetText(GetLocaleTimeString(fStartDateTime.Time_t()));
	fTextEndDate->SetText(GetLocaleDateString(fEndDateTime.Time_t()));
	fTextEndTime->SetText(GetLocaleTimeString(fEndDateTime.Time_t()));
}


void
EventWindow::DisableControls()
{
	fTextStartDate->SetEnabled(false);
	fTextEndDate->SetEnabled(false);
	fTextStartTime->SetEnabled(false);
	fTextEndTime->SetEnabled(false);
	fEveryMonth->SetEnabled(false);
	fEveryYear->SetEnabled(false);
	fStartTimeCheckBox->SetEnabled(false);
	fEndTimeCheckBox->SetEnabled(false);
	fCategoryMenuField->SetEnabled(false);
	fDeleteButton->SetEnabled(false);
}


BString
EventWindow::GetLocaleDateString(time_t timeValue)
{
	BString dateString;
	BDateFormat().Format(dateString, timeValue,
		B_SHORT_DATE_FORMAT);
	return dateString;
}


BString
EventWindow::GetLocaleTimeString(time_t timeValue)
{
	BString timeString;
	BTimeFormat().Format(timeString, timeValue,
		B_SHORT_TIME_FORMAT);
	return timeString;
}


bool
EventWindow::QuitRequested()
{
	((App*)be_app)->mainWindow()->PostMessage(kEventWindowQuitting);
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


void
EventWindow::OnSaveClick()
{
	if (BString(fTextName->Text()).CountChars() < 3) {

		BAlert* alert  = new BAlert("Error",
			"The name must have a length greater than 2",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
		return;
	}

	if (fAllDayCheckBox->Value() == B_CONTROL_OFF) {
		if (fStartDateTime > fEndDateTime) {
			BAlert* alert  = new BAlert("Error",
				"TInvalid range of time selected",
				NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

			alert->SetShortcut(0, B_ESCAPE);
			alert->Go();
			return;
		}
	}

	Event* newEvent = new Event(fTextName->Text(), fTextPlace->Text(),
		fTextDescription->Text(), fAllDayCheckBox->Value() == B_CONTROL_ON,
		fStartDateTime, fEndDateTime);

	if (fEvent!=NULL) {
		fEventList->RemoveItem(fEventIndex);
		fEventList->AddItem(newEvent, fEventIndex);
		CloseWindow();
	}

	else if (fEvent == NULL)
	{
		fEventList->AddItem(newEvent);
		delete newEvent;
		CloseWindow();
	}
}


void
EventWindow::OnDeleteClick()
{
	fEventList->RemoveItem(fEventIndex);
	CloseWindow();
}


void
EventWindow::CloseWindow()
{
	PostMessage(B_QUIT_REQUESTED);
}


void
EventWindow::OnCheckBoxToggle()
{

	if (fAllDayCheckBox->Value() == B_CONTROL_ON) {
		fTextStartTime->SetText("");
		fTextEndTime->SetText("");
	}

	else
	{
		fTextEndTime->SetText(GetLocaleTimeString(fEndDateTime.Time_t()));
		fTextStartTime->SetText(GetLocaleTimeString(fStartDateTime.Time_t()));
	}
}


Event*
EventWindow::GetEvent()
{
	return fEvent;
}