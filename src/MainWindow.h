/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <DateTime.h>
#include <Window.h>


class BList;
class BMenu;
class BMenuBar;
class DayView;
class EventWindow;
class MainView;
class Preferences;
class PreferenceWindow;
class SidePanelView;


namespace BPrivate {
	class BToolBar;
}
using BPrivate::BToolBar;


static const uint32 kMenuEditPref = 'kmap';
static const uint32 kMenuEditCategory = 'kmec';
static const uint32 kShowToday = 'ksty';
static const uint32 kAddEvent = 'kaet';


class MainWindow: public BWindow {
public:
			MainWindow();
	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

	void			SetPreferences(Preferences* preferences);

private:
	void			_LaunchEventManager(int32 index);
	void			_SyncWithPreferences();
	void			_UpdateDayView();
	BDate			_GetSelectedCalendarDate() const;

	static const int kMenuAppQuit	= 1000;
	static const int kDayView 	= 1002;
	static const int kMonthView	= 1003;

	MainView*	fMainView;
	EventWindow*	fEventWindow;
	BMenuBar*	fMenuBar;
	BMenu*		fAppMenu;
	BMenu*		fEventMenu;
	BMenu*		fCategoryMenu;
	BMenu*		fViewMenu;
	BToolBar*	fToolBar;
	SidePanelView*	fSidePanelView;
	DayView*	fDayView;
	BList*		fEventList;
	Preferences*	fPreferences;
};

#endif
