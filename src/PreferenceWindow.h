/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PREFWIN_H_
#define _PREFWIN_H_

#include <Window.h>

class BBox;
class BButton;
class BCheckBox;
class BPopUpMenu;
class BMenuField;
class BStringView;
class BView;
class Preferences;
class QueryDBManager;


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

		static const int		kStartOfWeekChangeMessage 				= 1000;
		static const int 		kShowWeekChangeMessage				= 1001;
		static const int		kApplyPreferencesMessage					= 1002;
		static const int		kRevertPreferencesMessage				= 1003;
		static const int		kDefaultCategoryChangeMessage 				= 1004;
		static const int 		kShowbottomMessage				= 1005;
		static const int 		kUseRightSide				= 1006;
		
		
		BView*				fMainView;
		BCheckBox*			fWeekNumberHeaderCB;
		BCheckBox*			fShowbottomVisibleCB;
		BCheckBox*			fUseRightSideCB;
		BStringView*			fWeekCategoryLabel;
		BStringView*			fOrgCategoryLabel;
		BStringView*			fStartOfWeekLabel;
		BStringView*			fDefaultCatLabel;
		BPopUpMenu*			fDayOfWeekMenu;
		BPopUpMenu*			fDefaultCatMenu;
		BMenuField*			fDayOfWeekMenuField;
		BMenuField*			fDefaultCatMenuField;
		BBox*				fWeekPreferencesBox;
		BBox*				fOrgPreferencesBox;
		BButton*			fApplyButton;
		BButton*			fRevertButton;

		Preferences*			fStartPreferences;
		Preferences*			fCurrentPreferences;
		Preferences*			fTempPreferences;

		QueryDBManager*		fDBManager;
};

#endif
