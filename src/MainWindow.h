/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <DateTime.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <private/shared/ToolBar.h>
#include <View.h>
#include <Window.h>

#include "SidePanelView.h"


class MainWindow: public BWindow {
public:
			MainWindow();
	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	static const int kMenuAppQuit	= 1000;
	static const int kShowToday 	= 1001;
	static const int kDayView 	= 1002;
	static const int kMonthView	= 1003;
	static const int kAddEvent	= 1004;
	
	BView*		fMainView;
	BMenuBar*	fMenuBar;
	BMenu*		fAppMenu;
	BToolBar*	fToolBar;
	BCalendarView*	fCalendarView;
	SidePanelView*	fSidePanelView;
};

#endif
