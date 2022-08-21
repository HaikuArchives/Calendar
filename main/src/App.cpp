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
#include <NodeInfo.h>

#include <locale.h>

#include "CategoryWindow.h"
//#include "EventSyncWindow.h"
#include "EventWindow.h"
#include "MainWindow.h"
#include "PreferenceWindow.h"
#include "Preferences.h"

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
	if (preferencesDir.InitCheck() == B_ENTRY_NOT_FOUND)
		preferencesDir.CreateDirectory(settingsPath.Path(), &preferencesDir);


	fPreferencesFile.SetTo(&preferencesDir, "settings");
	fPreferences = new Preferences();
	fPreferences->Load(fPreferencesFile.Path());
	fPreferences->fSettingsPath = settingsPath;

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
	const char* authors[] = {
		"Akshay Agarwal",
		"Humdinger",
		"Jaidyn Levesque",
		"malbx",
		"Fredrik Modéen",
		"Bach Nguyen",
		NULL
	};

	BAboutWindow* aboutW = new BAboutWindow(kAppName, kSignature);
	aboutW->AddDescription(
		B_TRANSLATE("A calendar application to manage your appointments."));
	aboutW->AddCopyright(2017, "Akshay Agarwal");
	aboutW->AddAuthors(authors);
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


Preferences*
App::GetPreferences()
{
	return fPreferences;
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
	switch (message->what) {

		case kMenuAppPref:
		{
			if (fPreferenceWindow == NULL) {
				fPreferenceWindow
					= new PreferenceWindow(fMainWindow->Frame(), fPreferences);
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

		/*case kMenuSyncGCAL:
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
			break;*/

		case kPreferenceWindowQuitting:
			fPreferenceWindow = NULL;
			break;

		case kAppPreferencesChanged:
			fMainWindow->PostMessage(message);
			break;

		case B_REFS_RECEIVED:
			RefsReceived(message);
			break;

		case B_LOCALE_CHANGED:
			fMainWindow->PostMessage(message);
			break;

		default:
		{
			BApplication::MessageReceived(message);
			break;
		}
	}
}


void
App::RefsReceived(BMessage* message)
{
	int i = 0;
	entry_ref ref;
	BFile file;
	BNodeInfo info;
	char type[B_FILE_NAME_LENGTH];

	while (message->HasRef("refs", i)) {
		BMessage msg = BMessage(B_REFS_RECEIVED);
		message->FindRef("refs", i++, &ref);
		msg.AddRef("refs", &ref);

		file.SetTo(&ref, B_READ_ONLY);
		info.SetTo(&file);
		info.GetType(type);

		if (BString(type) == BString("application/x-calendar-event")
			|| BString(type) == BString("text/calendar"))
			fMainWindow->PostMessage(&msg);
		else if (BString(type) == BString("application/x-calendar-category")) {
			MessageReceived(new BMessage(kMenuCategoryEdit));
			fCategoryWindow->PostMessage(&msg);
		}
	}
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	BMessage message(B_REFS_RECEIVED);

	for (int32 i = 1; i < argc; i++) {
		BEntry entry(argv[i]);
		entry_ref ref;
		if (entry.Exists() && entry.GetRef(&ref) == B_OK)
			message.AddRef("refs", &ref);
	}

	RefsReceived(&message);
}


int
main()
{
	setlocale(LC_ALL, "");

	App app;
	app.Run();
	return 0;
}
