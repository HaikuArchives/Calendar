/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef APP_H
#define APP_H


#include <Application.h>
#include <Path.h>


class CategoryWindow;
class MainWindow;
class Preferences;
class PreferenceWindow;
class EventSyncWindow;


extern const char* kAppName;
extern const char* kSignature;


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
	EventSyncWindow*	fEventSyncWindow;

	Preferences*		fPreferences;
	BPath			fPreferencesFile;
};

#endif
