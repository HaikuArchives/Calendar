/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <DateTime.h>
#include <Window.h>

class BMenu;
class BMenuBar;
class BSplitView;
class DayView;
class Event;
class EventWindow;
class MainView;
class Preferences;
class PreferenceWindow;
class SidePanelView;

namespace BPrivate {
	class BToolBar;
}
using BPrivate::BToolBar;


static const uint32 kMenuAppPref = 'kmap';
static const uint32 kMenuCategoryEdit = 'kmce';
static const uint32 kMenuSyncGCAL = 'kmsg';


class MainWindow: public BWindow {
public:
				MainWindow();
	virtual void		MessageReceived(BMessage* message);
	virtual bool		QuitRequested();

	void			StartNotificationThread();
	void			StopNotificationThread();
private:
	void			_InitInterface();
	void			_LaunchEventManager(Event* event);
	void			_SyncWithPreferences();
	void			_UpdateDayView();
	void			_SetEventListPopUpEnabled(bool state);
	BDate		_GetSelectedCalendarDate() const;
	void			_ToggleEventViewButton(int selectedButton);

	static const int	kMenuAppQuit		= 1000;
	static const int 	kMenuEventEdit 		= 1002;
	static const int 	kMenuEventDelete	= 1003;
	static const int 	kAddEvent 		= 1004;
	static const int 	kDayView 		= 1005;
	static const int 	kWeekView		= 1006;
	static const int 	kAgendaView		= 1007;

	MainView*		fMainView;
	EventWindow*		fEventWindow;
	BMenuBar*		fMenuBar;
	BMenu*			fAppMenu;
	BMenu*			fEventMenu;
	BMenu*			fCategoryMenu;
	BMenu*			fViewMenu;
	BMenu*			fSyncMenu;
	BToolBar*		fToolBar;
	SidePanelView*		fSidePanelView;
	DayView*		fDayView;
	thread_id		fNotificationThread;
};

#endif
