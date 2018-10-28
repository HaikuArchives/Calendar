/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EventWindow.h"

#include <time.h>

#include <Application.h>
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ColorMenuItem.h>
#include <DateFormat.h>
#include <File.h>
#include <GroupLayout.h>
#include <GraphicsDefs.h>
#include <LayoutItem.h>
#include <LayoutBuilder.h>
#include <List.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <Screen.h>
#include <TextControl.h>
#include <TextView.h>
#include <TimeFormat.h>
#include <View.h>

#include "App.h"
#include "CalendarMenuWindow.h"
#include "Category.h"
#include "CategoryEditWindow.h"
#include "Event.h"
#include "MainWindow.h"
#include "Preferences.h"
#include "SQLiteManager.h"


Preferences* EventWindow::fPreferences = NULL;


EventWindow::EventWindow()
	:
	BWindow(fPreferences->fEventWindowRect, "Event manager", B_TITLED_WINDOW,
			B_AUTO_UPDATE_SIZE_LIMITS)
{
	_InitInterface();

	if (fPreferences->fEventWindowRect == BRect()) {
		fPreferences->fEventWindowRect = Frame();
		CenterOnScreen();
	}

	_DisableControls();
}


EventWindow::~EventWindow()
{
	delete fDBManager;
	delete fCategoryList;
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
			OnDeleteClick();
			break;

		case kSavePressed:
			OnSaveClick();
			break;

		case kRefreshCategoryList:
		{	LockLooper();
			_UpdateCategoryMenu();
			UnlockLooper();
			break;
		}
		case kShowPopUpCalendar:
		{
			int8 which;
			message->FindInt8("which", &which);
			_ShowPopUpCalendar(which);
			break;
		}

		case kStartDateChanged:
		{
			BDate date;
			GetDateFromMessage(message, date);
			SetStartDate(date);
			break;
		}

		case kEndDateChanged:
		{
			BDate date;
			GetDateFromMessage(message, date);
			SetEndDate(date);
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
EventWindow::FrameMoved(BPoint newPosition)
{
	fPreferences->fEventWindowRect.OffsetTo(newPosition);
}


void
EventWindow::SetEvent(Event* event)
{
	fEvent = event;

	if (event != NULL) {
		fTextName->SetText(event->GetName());
		fTextPlace->SetText(event->GetPlace());
		fTextDescription->SetText(event->GetDescription());

		fStartDate = BDate(event->GetStartDateTime());
		fEndDate = BDate(event->GetEndDateTime());

		fTextStartDate->SetText(GetDateString(fStartDate));
		fTextEndDate->SetText(GetDateString(fEndDate));


		Category* category;

		for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
			category = ((Category*)fCategoryList->ItemAt(i));
			if (category->Equals(*event->GetCategory())) {
				fCategoryMenu->ItemAt(i)->SetMarked(true);
				break;
			}
		}

		if (event->IsAllDay()) {
			fAllDayCheckBox->SetValue(B_CONTROL_ON);
			fTextStartTime->SetEnabled(false);
			fTextEndTime->SetEnabled(false);
			fTextStartTime->SetText(GetLocaleTimeString(event->GetStartDateTime()));
			fTextEndTime->SetText(GetLocaleTimeString(event->GetEndDateTime()));
			// This is needed for week view
			fTextStartDate->SetText(GetDateString(fStartDate));
			fTextEndDate->SetText(GetDateString(fEndDate));
		}

		else
		{
			fAllDayCheckBox->SetValue(B_CONTROL_OFF);
			fTextStartTime->SetText(GetLocaleTimeString(event->GetStartDateTime()));
			fTextEndTime->SetText(GetLocaleTimeString(event->GetEndDateTime()));
		}


		fDeleteButton->SetEnabled(true);

	}

}


void
EventWindow::GetDateFromMessage(BMessage* message, BDate& date)
{
	int32 day, month, year;
	message->FindInt32("day", &day);
	message->FindInt32("month", &month);
	message->FindInt32("year", &year);
	date = BDate(year, month, day);
}


void
EventWindow::SetEventDate(BDate& date)
{
	// Set initial start time as (00:00) for a new event
	fStartDate = date;
	fTextStartDate->SetText(GetDateString(fStartDate));
	fTextStartTime->SetText(GetLocaleTimeString(BDateTime(fStartDate, BTime(0, 0, 0)).Time_t()));

	// Set initial end time as (01:00) for a new event
	fEndDate = date;
	fTextEndDate->SetText(GetDateString(fEndDate));
	fTextEndTime->SetText(GetLocaleTimeString(BDateTime(fStartDate, BTime(1, 0, 0)).Time_t()));
}


void
EventWindow::SetStartDate(BDate& date)
{
	if (!date.IsValid())
		return;
	fStartDate = date;
	fTextStartDate->SetText(GetDateString(fStartDate));
}


void
EventWindow::SetEndDate(BDate& date)
{
	if (!date.IsValid())
		return;
	fEndDate = date;
	fTextEndDate->SetText(GetDateString(fEndDate));
}


BString
EventWindow::GetDateString(BDate& date)
{
	BString dateString;
	BDateFormat().Format(dateString, date,
		B_SHORT_DATE_FORMAT);
	return dateString;
}


BString
EventWindow::GetLocaleTimeString(time_t timeValue)
{
	BString timeString;
	BTimeFormat timeFormat;
	timeFormat.SetTimeFormat(B_SHORT_TIME_FORMAT, "HH:mm");
	timeFormat.Format(timeString, timeValue,
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
EventWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


void
EventWindow::OnSaveClick()
{
	if (BString(fTextName->Text()).CountChars() < 3) {

		BAlert* alert  = new BAlert("Error",
			"The name must have a length greater than 2.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
		return;
	}

	time_t start;
	time_t end;
	BTime startTime;
	BTime endTime;

	if (fAllDayCheckBox->Value() == B_CONTROL_OFF) {
		BTimeFormat timeFormat;
		timeFormat.SetTimeFormat(B_SHORT_TIME_FORMAT, "HH:mm");
		timeFormat.Parse(fTextStartTime->Text(), B_SHORT_TIME_FORMAT, startTime);
		timeFormat.Parse(fTextEndTime->Text(), B_SHORT_TIME_FORMAT, endTime);
	}

	else
	{
		startTime.SetTime(0, 0, 0);
		endTime.SetTime(23, 59, 59, 59);
	}

	start = BDateTime(fStartDate, startTime).Time_t();
	end = BDateTime(fEndDate, endTime).Time_t();

	if (difftime(start, end) > 0) {
		BAlert* alert  = new BAlert("Error",
			"Sorry, you cannot create an event that ends before it starts.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

		alert->SetShortcut(0, B_ESCAPE);
		alert->Go();
		return;
	}

	Category* category = NULL;
	BMenuItem* item = fCategoryMenu->FindMarked();
	int32 index = fCategoryMenu->IndexOf(item);
	Category* c = ((Category*)fCategoryList->ItemAt(index));
	category = new Category(*c);

	bool notified = (difftime(start, BDateTime::CurrentDateTime(B_LOCAL_TIME).Time_t()) < 0) ? true : false;

	Event newEvent(fTextName->Text(), fTextPlace->Text(),
		fTextDescription->Text(), fAllDayCheckBox->Value() == B_CONTROL_ON,
		start, end, category, notified);

	if ((fEvent == NULL) && (fDBManager->AddEvent(&newEvent))) {
		CloseWindow();
	}

	else if ((fEvent != NULL) && (fDBManager->UpdateEvent(fEvent, &newEvent)))
	{
		CloseWindow();
	}

	else
	{
		BAlert* alert  = new BAlert("Error",
			"There was some error in adding the event. Please try again.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return;
	}
}


void
EventWindow::OnDeleteClick()
{
	BAlert* alert = new BAlert("Confirm delete",
		"Are you sure you want to delete this event?",
		NULL, "OK", "Cancel", B_WIDTH_AS_USUAL, B_WARNING_ALERT);

	alert->SetShortcut(1, B_ESCAPE);
	int32 button_index = alert->Go();

	if (button_index == 0) {
		Event newEvent(*fEvent);
		newEvent.SetStatus(false);
		newEvent.SetUpdated(time(NULL));
		fDBManager->UpdateEvent(fEvent, &newEvent);
		CloseWindow();
	}
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
		fTextStartTime->SetEnabled(false);
		fTextEndTime->SetEnabled(false);
	}

	else
	{
		fTextStartTime->SetText(GetLocaleTimeString(BDateTime(fStartDate, BTime(0, 0, 0)).Time_t()));
		fTextEndTime->SetText(GetLocaleTimeString(BDateTime(fEndDate, BTime(1, 0, 0)).Time_t()));
		fTextStartTime->SetEnabled(true);
		fTextEndTime->SetEnabled(true);

	}
}


void
EventWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fTextName = new BTextControl("EventName", NULL, NULL, NULL);
	fTextPlace = new BTextControl("EventPlace", NULL, NULL, NULL);
	fTextStartDate = new BTextControl("StartDate", NULL, NULL, NULL);
	fTextEndDate = new BTextControl("EndDate", NULL, NULL, NULL);
	fTextStartTime = new BTextControl("StartTime", NULL, NULL, NULL);
	fTextEndTime = new BTextControl("EndTime", NULL, NULL, NULL);

	const char* tooltip = "Enter the time in HH:mm (24 hour) format.";
	fTextStartTime->SetToolTip(tooltip);
	fTextEndTime->SetToolTip(tooltip);

	fTextDescription = new BTextView("TextDescription", B_WILL_DRAW);
	fTextDescription->MakeEditable();
	fTextDescription->SetExplicitMinSize(BSize(240, 100));

	fAllDayCheckBox = new BCheckBox("", new BMessage(kAllDayPressed));
	fAllDayCheckBox->SetValue(B_CONTROL_OFF);

	fEveryMonth = new BRadioButton("EveryMonth", "Monthly", new BMessage(kOptEveryMonth));
	fEveryYear = new BRadioButton("EveryYear", "Yearly", new BMessage(kOptEveryYear));

	fNameLabel = new BStringView("NameLabel", "Name:");
	fPlaceLabel = new BStringView("PlaceLabel", "Place:");
	fDescriptionLabel = new BStringView("DescriptionLabel", "Description:");
	fCategoryLabel = new BStringView("CategoryLabel", "Category:");
	fAllDayLabel = new BStringView("AllDayLabel", "All day:");
	fEndDateLabel = new BStringView("EndDateLabel", "End date:");
	fStartDateLabel = new BStringView("StartDateLabel", "Start date:");
	fStartTimeLabel = new BStringView("StartTimeLabel", "Start time:");
	fEndTimeLabel = new BStringView("EndTimeLabel", "End time:");

	fDeleteButton = new BButton("DeleteButton", "Delete", new BMessage(kDeletePressed));
	fDeleteButton->SetEnabled(false);
	BButton* CancelButton = new BButton("CancelButton", "Cancel", new BMessage(kCancelPressed));
	BButton* SaveButton = new BButton("SaveButton", "OK", new BMessage(kSavePressed));

	BMessage* message = new BMessage(kShowPopUpCalendar);
	message->AddInt8("which",0);
	fStartCalButton = new BButton("StartCalButton", "▼", message);
	message = new BMessage(kShowPopUpCalendar);
	message->AddInt8("which",1);
	fEndCalButton = new BButton("EndCalButton", "▼", message);

	float width, height;
	fStartDateLabel->GetPreferredSize(&width, &height);
	fStartCalButton->SetExplicitMinSize(BSize(height * 2, height));
	fEndCalButton->SetExplicitMinSize(BSize(height * 2, height));

	fDBManager = new SQLiteManager();

	fCategoryList = fDBManager->GetAllCategories();

	fCategoryMenu = new BMenu("CategoryMenu");
	Category* category;
	for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
		category = ((Category*)fCategoryList->ItemAt(i));
		fCategoryMenu->AddItem(new BColorMenuItem(category->GetName(), NULL, category->GetColor()));
	}
	fCategoryMenu->SetRadioMode(true);
	fCategoryMenu->SetLabelFromMarked(true);
	fCategoryMenu->ItemAt(0)->SetMarked(true);

	fStartDateEdit = new BMenu("Start date");
	fEndDateEdit = new BMenu("End date");

	fCategoryMenuField = new BMenuField("CategoryMenuField", NULL, fCategoryMenu);

	BBox* fRecurrenceBox = new BBox("RecurrenceBox");
	BLayoutBuilder::Group<>(fRecurrenceBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGroup(B_HORIZONTAL)
			.Add(fEveryMonth)
			.Add(fEveryYear)
		.End()
	.End();
	fRecurrenceBox->SetLabel("Recurrence");

	fStartDateBox = new BBox("startdatetime");
	BLayoutBuilder::Group<>(fStartDateBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGrid()
			.Add(fStartDateLabel, 0, 0)
			.Add(fTextStartDate, 1, 0)
			.Add(fStartCalButton, 2, 0)
			.Add(fStartTimeLabel, 0, 1)
			.Add(fTextStartTime, 1, 1)
		.End()
	.End();
	fStartDateBox->SetLabel("Start date and time");

	fEndDateBox = new BBox("enddatetime");
	BLayoutBuilder::Group<>(fEndDateBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_ITEM_INSETS)
		.AddStrut(B_USE_ITEM_SPACING)
		.AddGrid()
			.Add(fEndDateLabel, 0, 0)
			.Add(fTextEndDate, 1, 0)
			.Add(fEndCalButton, 2, 0)
			.Add(fEndTimeLabel, 0, 1)
			.Add(fTextEndTime, 1, 1)
		.End()
	.End();
	fEndDateBox->SetLabel("End date and time");

	BBox* divider = new BBox(BRect(0, 0, 1, 1),
		B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider->SetExplicitMaxSize(BSize(1, B_SIZE_UNLIMITED));

	BLayoutBuilder::Group<>(fMainView, B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING, 0,
			B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
		.AddGroup(B_VERTICAL)
			.AddGrid()
				.Add(fNameLabel, 0, 0)
				.Add(fTextName, 1, 0)
				.Add(fPlaceLabel, 0, 1)
				.Add(fTextPlace, 1 ,1)
			.End()
			.Add(fDescriptionLabel)
			.Add(fTextDescription)
			.AddGrid()
				.Add(fCategoryLabel, 0, 0)
				.Add(fCategoryMenuField, 1, 0)
			.End()
		.End()
		.Add(divider)
		.AddGroup(B_VERTICAL)
			.AddGrid()
				.SetInsets(B_USE_ITEM_INSETS, B_USE_ITEM_INSETS,
					B_USE_ITEM_INSETS, 0)
				.Add(fAllDayLabel, 0, 0)
				.Add(fAllDayCheckBox, 1 ,0)
			.End()
			.Add(fStartDateBox)
			.Add(fEndDateBox)
			.Add(fRecurrenceBox)
		.End()
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0, B_USE_DEFAULT_SPACING, 0, 0)
		.Add(fMainView)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.SetInsets(B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING,
				B_USE_DEFAULT_SPACING, B_USE_WINDOW_SPACING)
			.Add(fDeleteButton)
			.AddGlue()
			.Add(CancelButton)
			.Add(SaveButton)
		.End()
	.End();
}


void
EventWindow::_UpdateCategoryMenu()
{
	Category* selectedCategory = NULL;
	BMenuItem* item = fCategoryMenu->FindMarked();
	int32 index = fCategoryMenu->IndexOf(item);
	Category* c = ((Category*)fCategoryList->ItemAt(index));
	selectedCategory = new Category(*c);

	fCategoryList = fDBManager->GetAllCategories();

	Category* category;
	bool marked = false;

	fCategoryMenu->RemoveItems(0, fCategoryMenu->CountItems(), true);

	for (int32 i = 0; i < fCategoryList->CountItems(); i++) {
		category = ((Category*)fCategoryList->ItemAt(i));
		fCategoryMenu->AddItem(new BMenuItem(category->GetName(),
			new BMessage)); // TODO: Should this be sending a message?
		if (category->Equals(*selectedCategory) && (marked == false)) {
			fCategoryMenu->ItemAt(i)->SetMarked(true);
			marked = true;
		}
	}

	if(!marked)
		fCategoryMenu->ItemAt(0)->SetMarked(true);

	delete selectedCategory;
}


void
EventWindow::_DisableControls()
{
	fTextStartDate->SetEnabled(false);
	fTextEndDate->SetEnabled(false);
	fEveryMonth->SetEnabled(false);
	fEveryYear->SetEnabled(false);
}


void
EventWindow::_ShowPopUpCalendar(int8 which)
{
	if (fCalendarWindow.IsValid()) {
		BMessage activate(B_SET_PROPERTY);
		activate.AddSpecifier("Active");
		activate.AddBool("data", true);

		if (fCalendarWindow.SendMessage(&activate) == B_OK)
			return;
	}

	BPoint where;
	BPoint boxPosition;
	BPoint buttonPosition;
	BDate date;
	BMessage* invocationMessage;

	//TODO: This is bad. Improve how coordinates for pop up calendar
	//window is calculated. Better implement a DateTimeEdit control.

	if (which == 0) {
		boxPosition = fStartDateBox->Frame().RightTop();
		buttonPosition = fStartCalButton->Frame().LeftBottom();
		date = fStartDate;
		invocationMessage = new BMessage(kStartDateChanged);

	}
	else
	{
		boxPosition = fEndDateBox->Frame().RightTop();
		buttonPosition = fEndCalButton->Frame().LeftBottom();
		date = fEndDate;
		invocationMessage = new BMessage(kEndDateChanged);
	}

	where.x = boxPosition.x - buttonPosition.x;
	where.y = boxPosition.y + buttonPosition.y;
	where += BPoint(-62.0, 8.0);

	ConvertToScreen(&where);

	CalendarMenuWindow* window = new CalendarMenuWindow(this, where);
	window->SetDate(date);
	window->SetInvocationMessage(invocationMessage);
	window->SetDate(date);
	fCalendarWindow = BMessenger(window);
	window->Show();
}
