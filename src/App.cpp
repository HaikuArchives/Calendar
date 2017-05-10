/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "App.h"

#include <private/interface/AboutWindow.h>

const char* kAppName = "Calendar";
const char* kSignature = "application/x-vnd.calendar";


App::App()
	:
	BApplication(kSignature)
{
	fMainWindow = new MainWindow();
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


int
main()
{
	App app;
	app.Run();
	return 0;
}



