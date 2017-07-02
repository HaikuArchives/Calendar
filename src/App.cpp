/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "App.h"

#include <LocaleRoster.h>
#include <private/interface/AboutWindow.h>

#include <locale.h>

#include "EventWindow.h"
#include "CategoryWindow.h"

const char* kAppName = "Calendar";
const char* kSignature = "application/x-vnd.calendar";


App::App()
	:
	BApplication(kSignature),
	fEventWindow(NULL),
	fPreferenceWindow(NULL),
	fCategoryWindow(NULL)
{
	fMainWindow = new MainWindow();
	fPreferenceWindow = new PreferenceWindow();
	fMainWindow->Show();
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
	switch(message->what)
	{
		case kMenuEditPref:
		{
			fPreferenceWindow->Lock();
			if (fPreferenceWindow->IsHidden())
				fPreferenceWindow->Show();
			else
				fPreferenceWindow->Activate();
			fPreferenceWindow->Unlock();
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

		case kAddEvent:
		{
			if (fEventWindow == NULL) {
				fEventWindow = new EventWindow();
				fEventWindow->Show();
			}

			fEventWindow->Activate();
			break;
		}

		case kEventWindowQuitting:
			fEventWindow = NULL;
			break;

		case kCategoryWindowQuitting:
			fCategoryWindow = NULL;
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
