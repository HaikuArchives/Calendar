/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef APP_H
#define APP_H


#include <Application.h>

#include "MainWindow.h"


class App: public BApplication
{
public:
			App();
	void		AboutRequested();

private:
	MainWindow*	fMainWindow;
};

#endif
