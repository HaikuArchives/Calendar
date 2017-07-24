/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PREFWIN_H_
#define _PREFWIN_H_

#include <Box.h>
#include <CheckBox.h>
#include <DateFormat.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <StringView.h>
#include <View.h>
#include <Window.h>

#include "Preferences.h"


const uint32 kPreferenceWindowQuitting	= 'kpwq';
const uint32 kAppPreferencesChanged	= 'kpcd';


class PreferenceWindow : public BWindow {
public:
						PreferenceWindow(Preferences* preferences);
						~PreferenceWindow();

		void				MessageReceived(BMessage *message);
		bool				QuitRequested();

private:
		void				_InitInterface();
		void				_SyncPreferences(Preferences* preferences);
		void				_PreferencesModified();

		static const int		kStartOfWeekChangeMessage 	= 1000;
		static const int 		kShowWeekChangeMessage		= 1001;
		static const int		kApplyPreferencesMessage	= 1002;
		static const int		kRevertPreferencesMessage	= 1003;

		BView*				fMainView;
		BCheckBox*			fWeekNumberHeaderCB;
		BStringView*			fPrefCategoryLabel;
		BStringView*			fStartOfWeekLabel;
		BPopUpMenu*			fDayOfWeekMenu;
		BMenuField*			fDayOfWeekMenuField;
		BBox*				fWeekPreferencesBox;
		BButton*			fApplyButton;
		BButton*			fRevertButton;

		Preferences*			fStartPreferences;
		Preferences*			fCurrentPreferences;
		Preferences*			fTempPreferences;
};

#endif
