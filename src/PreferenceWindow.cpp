/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "PreferenceWindow.h"

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <StringView.h>
#include <View.h>
#include <Window.h>

#include "App.h"
#include "Preferences.h"
#include "SidePanelView.h"


PreferenceWindow::PreferenceWindow(Preferences* preferences)
	:BWindow(BRect(), "Preferences", B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE| B_AUTO_UPDATE_SIZE_LIMITS)
{
	fCurrentPreferences = preferences;

	fStartPreferences = new Preferences();
	*fStartPreferences = *fCurrentPreferences;

	fTempPreferences = new Preferences();
	*fTempPreferences = *fCurrentPreferences;

	_InitInterface();
	CenterOnScreen();

	_SyncPreferences(fCurrentPreferences);
}


PreferenceWindow::~PreferenceWindow()
{
	delete fStartPreferences;
	delete fTempPreferences;
}


void
PreferenceWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kStartOfWeekChangeMessage:
		{
			BMenuItem* item = fDayOfWeekMenu->FindMarked();
			int32 index = fDayOfWeekMenu->IndexOf(item);
			fTempPreferences->fStartOfWeekOffset = index;
			_PreferencesModified();
			break;
		}

		case kShowWeekChangeMessage:
		{
			bool state = fWeekNumberHeaderCB->Value() == B_CONTROL_ON;
			fTempPreferences->fHeaderVisible = state;
			_PreferencesModified();
			break;
		}

		case kApplyPreferencesMessage:
		{
			*fCurrentPreferences = *fTempPreferences;
			fApplyButton->SetEnabled(false);
			BMessage changed(kAppPreferencesChanged);
			be_app->PostMessage(&changed);
			break;
		}

		case kRevertPreferencesMessage:
		{
			*fTempPreferences = *fStartPreferences;
			fRevertButton->SetEnabled(false);
			fApplyButton->SetEnabled(true);
			_SyncPreferences(fTempPreferences);
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
	be_app->PostMessage(kPreferenceWindowQuitting);
	return true;
}


void
PreferenceWindow::_InitInterface()
{
	fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fDayOfWeekMenu = new BPopUpMenu("DayOfWeekMenu");

	const char* startOfWeekItems[] = {"Locale based", "Monday",
	"Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", NULL};
	for (int i = 0; startOfWeekItems[i]; ++i)
		fDayOfWeekMenu->AddItem(new BMenuItem(startOfWeekItems[i],
			new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->ItemAt(0)->SetMarked(true);
	fDayOfWeekMenuField = new BMenuField("DayOfWeekMenu", NULL, fDayOfWeekMenu);

	fPrefCategoryLabel = new BStringView("PrefCategory", "Week");
	BFont font;
	fPrefCategoryLabel->GetFont(&font);
	font.SetSize(font.Size() * 1.2);
	font.SetFace(B_BOLD_FACE);
	fPrefCategoryLabel->SetFont(&font, B_FONT_ALL);

	fStartOfWeekLabel = new BStringView("StartOfWeek", "First day of week");

	fWeekNumberHeaderCB = new BCheckBox("WeekNumberHeader",
		"Show week number in Calendar", new BMessage(kShowWeekChangeMessage));
	fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);

	fApplyButton = new BButton("Apply", new BMessage(kApplyPreferencesMessage));
	fRevertButton = new BButton("Revert", new BMessage(kRevertPreferencesMessage));

	fApplyButton->SetEnabled(false);
	fRevertButton->SetEnabled(false);

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
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
				.Add(fRevertButton)
				.AddGlue()
				.Add(fApplyButton)
			.End()
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
			.Add(fMainView)
	.End();
}


void
PreferenceWindow::_SyncPreferences(Preferences* preferences)
{
	if(preferences->fHeaderVisible == true) {
		fWeekNumberHeaderCB->SetValue(B_CONTROL_ON);
	} else {
		fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);
	}

	BMenuItem* item = fDayOfWeekMenu->ItemAt(preferences->fStartOfWeekOffset);
	item->SetMarked(true);
}


void
PreferenceWindow::_PreferencesModified()
{
	fApplyButton->SetEnabled(true);
	fRevertButton->SetEnabled(true);
}

