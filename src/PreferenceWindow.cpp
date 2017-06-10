/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "PreferenceWindow.h"

#include <Application.h>
#include <DateFormat.h>
#include <LayoutBuilder.h>

#include "App.h"
#include "SidePanelView.h"


PreferenceWindow::PreferenceWindow()
	:BWindow(BRect(), "Preferences", B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE| B_AUTO_UPDATE_SIZE_LIMITS)
{
	CenterOnScreen();

	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fDayOfWeekMenu = new BMenu("DayOfWeekMenu");

	fDayOfWeekMenu->AddItem(new BMenuItem("Locale Based", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Monday", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Tuesday", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Wednesday", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Thursday", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Friday", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Saturday", new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->AddItem(new BMenuItem("Sunday", new BMessage(kStartOfWeekChangeMessage)));

	fDayOfWeekMenu->SetRadioMode(true);
	fDayOfWeekMenu->SetLabelFromMarked(true);
	fDayOfWeekMenu->SetRadioMode(true);
	fDayOfWeekMenu->ItemAt(0)->SetMarked(true);

	fDayOfWeekMenuField = new BMenuField("DayOfWeekMenu", NULL, fDayOfWeekMenu);


	fPrefCategoryLabel = new BStringView("PrefCategory", "Week");
	BFont font;
	fPrefCategoryLabel->GetFont(&font);
	font.SetSize(font.Size() * 1.2);
	font.SetFace(B_BOLD_FACE);
	fPrefCategoryLabel->SetFont(&font, B_FONT_ALL);

	fStartOfWeekLabel = new BStringView("StartOfWeek", "First Day Of Week");

	fWeekNumberHeaderCB = new BCheckBox("WeekNumberHeader",
		"Show Week Number in Calendar", new BMessage(kShowWeekChangeMessage));
	fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);

	fWeekPreferencesBox = new BBox("Week");

	BLayoutBuilder::Group<>(fWeekPreferencesBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.SetInsets(B_USE_ITEM_INSETS)
			.AddStrut(B_USE_ITEM_SPACING)
			.Add(fStartOfWeekLabel)
			.Add(fDayOfWeekMenuField)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.Add(fWeekNumberHeaderCB)
	.End();
	fWeekPreferencesBox->SetLabel(fPrefCategoryLabel);

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_DEFAULT_SPACING)
			.SetInsets(B_USE_SMALL_INSETS)
			.Add(fWeekPreferencesBox)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
			.Add(fMainView)
	.End();

}


void
PreferenceWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kStartOfWeekChangeMessage:
		{	BMenuItem* item = fDayOfWeekMenu->FindMarked();
			int32 index = fDayOfWeekMenu->IndexOf(item);
			BMessage msg(kSetStartOfWeekMessage);
			msg.AddInt32("weekday", index);
			((App*)be_app)->mainWindow()->PostMessage(&msg);
			break;
		}

		case kShowWeekChangeMessage:
		{
			bool state = fWeekNumberHeaderCB->Value()==B_CONTROL_ON;
			BMessage msg(kShowWeekNumberMessage);
			msg.AddBool("state", state);
			((App*)be_app)->mainWindow()->PostMessage(&msg);
			break;

		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
PreferenceWindow::QuitRequested()
{
	Hide();
	return false;
}


void
PreferenceWindow::SetWeekHeader(bool state)
{
	fWeekNumberHeaderCB->SetValue(state ? B_CONTROL_ON : B_CONTROL_OFF);
}


bool
PreferenceWindow::IsWeekHeaderEnabled()
{
	return fWeekNumberHeaderCB->Value() == B_CONTROL_ON;
}


void
PreferenceWindow::SetStartOfWeek(int index)
{
	BMenuItem* item = fDayOfWeekMenu->ItemAt(index);
	item->SetMarked(true);
}


int
PreferenceWindow::GetStartOfWeek()
{
	BMenuItem* item = fDayOfWeekMenu->FindMarked();
	int index = fDayOfWeekMenu->IndexOf(item);
	return index;
}
