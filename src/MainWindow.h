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
class Event;
class EventTabView;
class EventWindow;
class MainView;
class Preferences;
class PreferenceWindow;
class SidePanelView;


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
	void			_LaunchEventManager(Event* event, entry_ref* ref = NULL);
	void			_SyncWithPreferences();
	void			_UpdateEventsView();
	void			_SetEventListPopUpEnabled(bool state);
	BDate			_GetSelectedCalendarDate() const;
	void			_ToggleEventMenu(BMessage* msg);

	static const int	kMenuAppQuit		= 1000;
	static const int 	kAddEvent 		= 1005;


	MainView*		fMainView;
	EventWindow*	fEventWindow;
	BMenuBar*		fMenuBar;
	BMenu*			fAppMenu;
	BMenu*			fEventMenu;
	BMenu*			fCategoryMenu;
	BMenu*			fViewMenu;
	BMenu*			fSyncMenu;
	SidePanelView*	fSidePanelView;
	EventTabView*	fEventsView;
	thread_id		fNotificationThread;
};

#endif
