/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "App.h"

#include <AboutWindow.h>
#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <LocaleRoster.h>

#include <locale.h>

#include "EventWindow.h"
#include "EventSyncWindow.h"
#include "CategoryWindow.h"
#include "MainWindow.h"
#include "Preferences.h"
#include "PreferenceWindow.h"

const char* kAppName = B_TRANSLATE_SYSTEM_NAME("Calendar");
const char* kSignature = "application/x-vnd.calendar";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"

App::App()
	:
	BApplication(kSignature),
	fPreferenceWindow(NULL),
	fCategoryWindow(NULL),
	fPreferences(NULL),
	fEventSyncWindow(NULL)
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

	MainWindow::SetPreferences(fPreferences);
	EventWindow::SetPreferences(fPreferences);

	fMainWindow = new MainWindow();
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
	aboutW->AddDescription(B_TRANSLATE("A native Calendar application for Haiku."));
	aboutW->AddCopyright(2017, "Akshay Agarwal");
	aboutW->SetVersion("1.0");
	aboutW->Show();
}


bool
App::QuitRequested()
{
	fPreferences->fMainWindowRect = fMainWindow->Frame();

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

		case kMenuAppPref:
		{
			if (fPreferenceWindow == NULL) {
				fPreferenceWindow = new PreferenceWindow(fPreferences);
				fPreferenceWindow->Show();
			}

			fPreferenceWindow->Activate();
			break;
		}

		case kMenuCategoryEdit:
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

		case kMenuSyncGCAL:
		{
			if (fEventSyncWindow == NULL) {
				fEventSyncWindow = new EventSyncWindow();
				fEventSyncWindow->Show();
			}

			fEventSyncWindow->Activate();
			break;
		}

		case kEventSyncWindowQuitting:
			fEventSyncWindow = NULL;
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
