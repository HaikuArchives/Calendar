/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef APP_H
#define APP_H

#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

#include "CategoryWindow.h"
#include "MainWindow.h"
#include "Preferences.h"
#include "PreferenceWindow.h"


class App: public BApplication
{
public:
				App();
				~App();

	void			AboutRequested();
	bool			QuitRequested();
	void			MessageReceived(BMessage* message);
	MainWindow*		mainWindow();
	CategoryWindow*		categoryWindow();


private:
	MainWindow*		fMainWindow;
	PreferenceWindow*	fPreferenceWindow;
	CategoryWindow*		fCategoryWindow;
	Preferences*		fPreferences;
	BPath			fPreferencesFile;
};

#endif
