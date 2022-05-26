/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PREFWIN_H_
#define _PREFWIN_H_

#include <Window.h>

class BButton;
class BCheckBox;
class BPopUpMenu;
class BMenuField;
class Preferences;
class QueryDBManager;


const uint32 kPreferenceWindowQuitting = 'kpwq';
const uint32 kAppPreferencesChanged = 'kpcd';


class PreferenceWindow : public BWindow
{
public:
					PreferenceWindow(BRect position, Preferences* preferences);
					~PreferenceWindow();

	void			MessageReceived(BMessage* message);
	bool			QuitRequested();

private:
	void			_InitInterface();
	void			_SyncPreferences(Preferences* preferences);
	void			_PreferencesModified();

	static const int kStartOfWeekChangeMessage = 1000;
	static const int kShowWeekChangeMessage = 1001;
	static const int kApplyPreferencesMessage = 1002;
	static const int kRevertPreferencesMessage = 1003;
	static const int kDefaultCategoryChangeMessage = 1004;

	BCheckBox*		fWeekNumberHeaderCB;
	BPopUpMenu*		fDayOfWeekMenu;
	BPopUpMenu*		fDefaultCatMenu;
	BMenuField*		fDayOfWeekMenuField;
	BMenuField*		fDefaultCatMenuField;
	BButton*		fApplyButton;
	BButton*		fRevertButton;

	Preferences*	fStartPreferences;
	Preferences*	fCurrentPreferences;
	Preferences*	fTempPreferences;

	QueryDBManager*	fDBManager;
};

#endif
