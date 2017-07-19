/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "App.h"

#include <LocaleRoster.h>
#include <AboutWindow.h>

#include <locale.h>

#include "EventWindow.h"
#include "CategoryWindow.h"

const char* kAppName = "Calendar";
const char* kSignature = "application/x-vnd.calendar";


App::App()
	:
	BApplication(kSignature),
	fPreferenceWindow(NULL),
	fCategoryWindow(NULL),
	fPreferences(NULL)
{
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(kAppName);
	BDirectory preferencesDir(settingsPath.Path());
	if(preferencesDir.InitCheck() == B_ENTRY_NOT_FOUND) {
		preferencesDir.CreateDirectory(settingsPath.Path(), &preferencesDir);
	}

	fPreferencesFile.SetTo(&preferencesDir, "settings");
	fPreferences = new Preferences();
	fPreferences->Load(fPreferencesFile.Path());
	fPreferences->fSettingsPath = settingsPath;

	fMainWindow = new MainWindow();
	fMainWindow->SetPreferences(fPreferences);
	fMainWindow->Show();
}


App::~App()
{
	fPreferences->Save(fPreferencesFile.Path());
	delete fPreferences;
}


void
App::AboutRequested()
{
	BAboutWindow* aboutW = new BAboutWindow(kAppName, kSignature);
	aboutW->AddDescription("A native Calendar application for Haiku.");
	aboutW->AddCopyright(2017, "Akshay Agarwal");
	aboutW->SetVersion("1.0");
	aboutW->Show();
}


bool
App::QuitRequested()
{
	if (fMainWindow->Lock())
		fMainWindow->Quit();
	return true;
}


MainWindow*
App::mainWindow()
{
	return fMainWindow;
}


CategoryWindow*
App::categoryWindow()
{
	return fCategoryWindow;
}


void
App::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kMenuEditPref:
		{
			if (fPreferenceWindow == NULL) {
				fPreferenceWindow = new PreferenceWindow(fPreferences);
				fPreferenceWindow->Show();
			}

			fPreferenceWindow->Activate();
			break;
		}

		case kMenuEditCategory:
		{
			if (fCategoryWindow == NULL) {
				fCategoryWindow = new CategoryWindow();
				fCategoryWindow->Show();
			}

			fCategoryWindow->Activate();
			break;
		}

		case kCategoryWindowQuitting:
			fCategoryWindow = NULL;
			break;

		case kPreferenceWindowQuitting:
			fPreferenceWindow = NULL;
			break;

		case kAppPreferencesChanged:
			fMainWindow->PostMessage(message);
			break;

		case B_LOCALE_CHANGED:
			fMainWindow->PostMessage(message);
			break;

		default: {
			BApplication::MessageReceived(message);
			break;
		}
	}
}


int
main()
{
	setlocale(LC_ALL, "");

	App app;
	app.Run();
	return 0;
}
