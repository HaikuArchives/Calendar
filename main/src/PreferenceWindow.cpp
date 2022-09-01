/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 * Contributed by:
 *	Humdinger <humdingerb@gmail.com>, 2021
 */


#include "PreferenceWindow.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ColorMenuItem.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <SeparatorView.h>
#include <Window.h>

#include "App.h"
#include "Category.h"
#include "Preferences.h"
#include "QueryDBManager.h"
#include "SidePanelView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferenceWindow"


PreferenceWindow::PreferenceWindow(BRect frame, Preferences* preferences)
	:
	BWindow(BRect(), B_TRANSLATE("Settings"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fCurrentPreferences = preferences;

	fStartPreferences = new Preferences();
	*fStartPreferences = *fCurrentPreferences;

	fTempPreferences = new Preferences();
	*fTempPreferences = *fCurrentPreferences;

	fDBManager = new QueryDBManager(((App*) be_app)->GetPreferences()->fDefaultCategory);

	_InitInterface();

	CenterIn(frame);
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
	switch (message->what) {

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

		case kDefaultCategoryChangeMessage:
		{
			BMenuItem* item = fDefaultCatMenu->FindMarked();
			fTempPreferences->fDefaultCategory = BString(item->Label());
			_PreferencesModified();
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
	fDayOfWeekMenu = new BPopUpMenu("DayOfWeekMenu");
	fDefaultCatMenu = new BPopUpMenu("DefaultCatMenu");

	const char* startOfWeekItems[] = {B_TRANSLATE("Locale based"),
		B_TRANSLATE("Monday"), B_TRANSLATE("Tuesday"), B_TRANSLATE("Wednesday"),
		B_TRANSLATE("Thursday"), B_TRANSLATE("Friday"), B_TRANSLATE("Saturday"),
		B_TRANSLATE("Sunday"), NULL};
	for (int i = 0; startOfWeekItems[i]; ++i)
		fDayOfWeekMenu->AddItem(new BMenuItem(
			startOfWeekItems[i], new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->ItemAt(0)->SetMarked(true);
	fDayOfWeekMenuField = new BMenuField(
		"DayOfWeekMenu", B_TRANSLATE("First day of the week:"), fDayOfWeekMenu);

	CategoryList* categories = fDBManager->GetAllCategories();
	for (int i = 0; i < categories->CountItems(); i++) {
		Category* category = categories->ItemAt(i);
		fDefaultCatMenu->AddItem(new BColorMenuItem(category->GetName(),
			new BMessage(kDefaultCategoryChangeMessage), category->GetColor()));
	}
	fDefaultCatMenu->ItemAt(0)->SetMarked(true);
	fDefaultCatMenuField = new BMenuField(
		"DefaultCatMenu", B_TRANSLATE("Default category:"), fDefaultCatMenu);

	fWeekNumberHeaderCB = new BCheckBox("WeekNumberHeader",
		B_TRANSLATE("Show week numbers in calendar"),
		new BMessage(kShowWeekChangeMessage));
	fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);

	fApplyButton = new BButton(
		B_TRANSLATE("Apply"), new BMessage(kApplyPreferencesMessage));
	fRevertButton = new BButton(
		B_TRANSLATE("Revert"), new BMessage(kRevertPreferencesMessage));

	fApplyButton->SetEnabled(false);
	fRevertButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_SMALL_SPACING)
		.SetInsets(
			B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, 0)
		.AddMenuField(fDefaultCatMenuField, 0, 0)
		.AddMenuField(fDayOfWeekMenuField, 0, 1)
		.Add(fWeekNumberHeaderCB, 0, 2, 2)
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(
			B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
		.Add(fRevertButton)
		.AddGlue()
		.Add(fApplyButton)
		.End()
		.End();
}


void
PreferenceWindow::_SyncPreferences(Preferences* preferences)
{
	if (preferences->fHeaderVisible == true)
		fWeekNumberHeaderCB->SetValue(B_CONTROL_ON);
	else
		fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);

	BMenuItem* item = fDayOfWeekMenu->ItemAt(preferences->fStartOfWeekOffset);
	item->SetMarked(true);
}


void
PreferenceWindow::_PreferencesModified()
{
	fApplyButton->SetEnabled(true);
	fRevertButton->SetEnabled(true);
}
