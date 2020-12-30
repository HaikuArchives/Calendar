/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "PreferenceWindow.h"

#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <StringView.h>
#include <View.h>
#include <Window.h>

#include "App.h"
#include "Category.h"
#include "Preferences.h"
#include "QueryDBManager.h"
#include "SidePanelView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferenceWindow"

PreferenceWindow::PreferenceWindow(Preferences* preferences)
	:BWindow(BRect(), B_TRANSLATE("Preferences"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE| B_AUTO_UPDATE_SIZE_LIMITS)
{
	fCurrentPreferences = preferences;

	fStartPreferences = new Preferences();
	*fStartPreferences = *fCurrentPreferences;

	fTempPreferences = new Preferences();
	*fTempPreferences = *fCurrentPreferences;

	fDBManager = new QueryDBManager();

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

		case kDefaultCategoryChangeMessage:
		{
			BMenuItem* item = fDefaultCatMenu->FindMarked();
			fTempPreferences->fDefaultCategory = BString(item->Label());
			_PreferencesModified();
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
	fDefaultCatMenu = new BPopUpMenu("DefaultCatMenu");

	const char* startOfWeekItems[] = {B_TRANSLATE("Locale based"),
		B_TRANSLATE("Monday"), B_TRANSLATE("Tuesday"),
		B_TRANSLATE("Wednesday"), B_TRANSLATE("Thursday"),
		B_TRANSLATE("Friday"), B_TRANSLATE("Saturday"),
		B_TRANSLATE("Sunday"), NULL};
	for (int i = 0; startOfWeekItems[i]; ++i)
		fDayOfWeekMenu->AddItem(new BMenuItem(startOfWeekItems[i],
			new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->ItemAt(0)->SetMarked(true);
	fDayOfWeekMenuField = new BMenuField("DayOfWeekMenu", NULL, fDayOfWeekMenu);

	BList* categories = fDBManager->GetAllCategories();
	for (int i = 0; i < categories->CountItems(); i++) {
		Category* category = (Category*)categories->ItemAt(i);
		fDefaultCatMenu->AddItem(new BMenuItem(category->GetName(),
			new BMessage(kDefaultCategoryChangeMessage)));
	}
	fDefaultCatMenu->ItemAt(0)->SetMarked(true);
	fDefaultCatMenuField = new BMenuField("DefaultCatMenu", NULL, fDefaultCatMenu);

	fWeekCategoryLabel = new BStringView("PrefCategory", "Week");
	fOrgCategoryLabel = new BStringView("PrefCategory", "Organization");
	BFont font;
	fWeekCategoryLabel->GetFont(&font);
	font.SetSize(font.Size() * 1.2);
	font.SetFace(B_BOLD_FACE);
	fWeekCategoryLabel->SetFont(&font, B_FONT_ALL);
	fOrgCategoryLabel->SetFont(&font, B_FONT_ALL);

	fStartOfWeekLabel = new BStringView("StartOfWeek", B_TRANSLATE("First day of week"));
	fDefaultCatLabel = new BStringView("DefaultCat", B_TRANSLATE("Default category"));

	fWeekNumberHeaderCB = new BCheckBox("WeekNumberHeader",
		B_TRANSLATE("Show week number in Calendar"), new BMessage(kShowWeekChangeMessage));
	fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);

	fApplyButton = new BButton(B_TRANSLATE("Apply"), new BMessage(kApplyPreferencesMessage));
	fRevertButton = new BButton(B_TRANSLATE("Revert"), new BMessage(kRevertPreferencesMessage));

	fApplyButton->SetEnabled(false);
	fRevertButton->SetEnabled(false);

	fWeekPreferencesBox = new BBox("Week");
	fOrgPreferencesBox = new BBox("Organization");

	BLayoutBuilder::Group<>(fWeekPreferencesBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.SetInsets(B_USE_ITEM_INSETS)
			.AddStrut(B_USE_ITEM_SPACING)
			.Add(fStartOfWeekLabel)
			.Add(fDayOfWeekMenuField)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.Add(fWeekNumberHeaderCB)
	.End();
	fWeekPreferencesBox->SetLabel(fWeekCategoryLabel);

	BLayoutBuilder::Group<>(fOrgPreferencesBox, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
			.SetInsets(B_USE_ITEM_INSETS)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.Add(fDefaultCatLabel)
			.Add(fDefaultCatMenuField)
	.End();
	fOrgPreferencesBox->SetLabel(fOrgCategoryLabel);


	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_DEFAULT_SPACING)
			.SetInsets(B_USE_SMALL_INSETS)
			.Add(fWeekPreferencesBox)
			.Add(fOrgPreferencesBox)
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

