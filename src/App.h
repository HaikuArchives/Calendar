/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef APP_H
#define APP_H

#include <Application.h>

#include "CategoryWindow.h"
#include "EventWindow.h"
#include "MainWindow.h"
#include "PreferenceWindow.h"


class App: public BApplication
{
public:
				App();

	void			AboutRequested();
	bool			QuitRequested();
	void			MessageReceived(BMessage* message);
	MainWindow*		mainWindow();
	CategoryWindow*	categoryWindow();


private:
	MainWindow*		fMainWindow;
	EventWindow*		fEventWindow;
	PreferenceWindow*	fPreferenceWindow;
	CategoryWindow*		fCategoryWindow;
};

#endif
