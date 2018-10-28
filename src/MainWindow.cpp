/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MainWindow.h"

#include <Application.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <ToolBar.h>

#include "CategoryEditWindow.h"
#include "DayView.h"
#include "Event.h"
#include "EventListView.h"
#include "EventSyncWindow.h"
#include "EventWindow.h"
#include "MainView.h"
#include "Preferences.h"
#include "PreferenceWindow.h"
#include "ResourceLoader.h"
#include "SidePanelView.h"


using BPrivate::BToolBar;


extern int32 NotificationLoop(void* data);
Preferences* MainWindow::fPreferences = NULL;


MainWindow::MainWindow()
	:
	BWindow(fPreferences->fMainWindowRect, "Calendar", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fEventWindow(NULL)
{
	SetPulseRate(500000);

	_InitInterface();

	if (fPreferences->fMainWindowRect == BRect()) {
		ResizeTo(640, 360);
		CenterOnScreen();
	}

	_SyncWithPreferences();
	fNotificationThread = -1;
	StartNotificationThread();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return false;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kMenuAppQuit:
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;

		case kAddEvent:
			_LaunchEventManager(NULL);
			break;

		case kMenuEventEdit:
		{
			BMessage msg(kEditEventMessage);
			fDayView->MessageReceived(&msg);
			break;
		}

		case kMenuEventDelete:
		{
			BMessage msg(kDeleteEventMessage);
			fDayView->MessageReceived(&msg);
			break;
		}
		
		case kWeekView: case kDayView:
		{
			fDayView->MessageReceived(message);
			break;
		}

		case kLaunchEventManagerToModify:
		{
			Event* event;
			message->FindPointer("event", (void**)(&event));
			_LaunchEventManager(event);
			break;
		}

		case kEventWindowQuitting:
		{
			fEventWindow = NULL;
			_UpdateDayView();
			_SetEventListPopUpEnabled(true);
			fEventMenu->SetEnabled(true);
			break;
		}

		case kSetCalendarToCurrentDate:
			fSidePanelView->MessageReceived(message);
			break;

		case kSelectedDateChanged:
			_UpdateDayView();
			break;

		case kSelectionMessage:
		{
			fSidePanelView->MessageReceived(message);
			break;
		}

		case kMonthUpMessage:
		case kMonthDownMessage:
			fSidePanelView->MessageReceived(message);
			break;

		case B_LOCALE_CHANGED:
		{	fSidePanelView->MessageReceived(message);
			fSidePanelView->SetStartOfWeek(fPreferences->fStartOfWeekOffset);
			_UpdateDayView();
			break;
		}

		case kSynchronizationComplete:
			_UpdateDayView();
			break;

		case kAppPreferencesChanged:
			_SyncWithPreferences();
			break;

		case kMenuAppPref:
			be_app->PostMessage(message);
			break;

		case kMenuCategoryEdit:
			be_app->PostMessage(message);
			break;

		case kMenuSyncGCAL:
			be_app->PostMessage(message);
			break;

		case kRefreshCategoryList:
		{
			_UpdateDayView();
			if (fEventWindow != NULL) {
				BMessenger msgr(fEventWindow);
				msgr.SendMessage(message);
			}
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
MainWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
}


void
MainWindow::StartNotificationThread()
{
	if (fNotificationThread < 0) {
		fNotificationThread = spawn_thread(NotificationLoop, "Notification thread",
			B_NORMAL_PRIORITY, NULL);
		resume_thread(fNotificationThread);
	}
}


void
MainWindow::StopNotificationThread()
{
	if (fNotificationThread > 0) {
		kill_thread(fNotificationThread);
		fNotificationThread = -1;
	}
}

void
MainWindow::_InitInterface()
{
	fMainView = new MainView();

	fMenuBar = new BMenuBar("MenuBar");

	fAppMenu = new BMenu("App");
	BMenuItem* item = new BMenuItem("About", new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	fAppMenu->AddItem(item);
	fAppMenu->AddItem(new BMenuItem("Preferences", new BMessage(kMenuAppPref)));
	fSyncMenu = new BMenu("Synchronize");
	fSyncMenu->AddItem(new BMenuItem("Google Calendar", new BMessage(kMenuSyncGCAL)));
	fAppMenu->AddItem(fSyncMenu);
	fAppMenu->AddSeparatorItem();
	fAppMenu->AddItem(new BMenuItem("Quit", new BMessage(kMenuAppQuit), 'Q', B_COMMAND_KEY));

	fEventMenu = new BMenu("Event");
	fEventMenu->AddItem(new BMenuItem("Add event", new BMessage(kAddEvent)));
	fEventMenu->AddItem(new BMenuItem("Edit event", new BMessage(kMenuEventEdit)));
	fEventMenu->AddItem(new BMenuItem("Remove event", new BMessage(kMenuEventDelete)));

	fCategoryMenu = new BMenu("Category");
	fCategoryMenu->AddItem(new BMenuItem("Edit categories", new BMessage(kMenuCategoryEdit)));
	fViewMenu = new BMenu("View");
	fViewMenu->AddItem(new BMenuItem("Day view", new BMessage(kDayView)));
	fViewMenu->AddItem(new BMenuItem("Week view", new BMessage(kWeekView)));
	fViewMenu->AddSeparatorItem();
	fViewMenu->AddItem(new BMenuItem("Go to today", new BMessage(kSetCalendarToCurrentDate)));

	fMenuBar->AddItem(fAppMenu);
	fMenuBar->AddItem(fEventMenu);
	fMenuBar->AddItem(fCategoryMenu);
	fMenuBar->AddItem(fViewMenu);

	fToolBar = new BToolBar();
	fToolBar->AddAction(new BMessage(kSetCalendarToCurrentDate), this, LoadVectorIcon("CALENDAR_ICON"),
		"Today", "Today", true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kDayView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Day view", "Day view", true);
	fToolBar->AddAction(new BMessage(kWeekView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Week view", "Week view", true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kAddEvent), this, LoadVectorIcon("ADD_EVENT"),
		"Add event", "Add event", true);
	fToolBar->AddGlue();

	fSidePanelView = new SidePanelView();
	fDayView = new DayView(BDate::CurrentDate(B_LOCAL_TIME));

	fMainView->StartWatchingAll(fSidePanelView);

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, 0.0f)
		.Add(fDayView)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.Add(fMenuBar)
		.Add(fToolBar)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fMainView, 5)
			.Add(fSidePanelView, 1)
		.End()
	.End();
}


void
MainWindow::_LaunchEventManager(Event* event)
{
	if (fEventWindow == NULL) {
		fEventWindow = new EventWindow();
		fEventWindow->SetEvent(event);

		if (event == NULL) {
			BDate date = _GetSelectedCalendarDate();
			fEventWindow->SetEventDate(date);
		}

		fEventWindow->Show();
		_SetEventListPopUpEnabled(false);
		fEventMenu->SetEnabled(false);
	}

	fEventWindow->Activate();
}


void
MainWindow::_SetEventListPopUpEnabled(bool state)
{
	fDayView->SetEventListPopUpEnabled(state);
}


void
MainWindow::_SyncWithPreferences()
{
	if (fPreferences != NULL) {
		if(fPreferences->fHeaderVisible == true)
			fSidePanelView->ShowWeekHeader(true);
		else
			fSidePanelView->ShowWeekHeader(false);

		fSidePanelView->SetStartOfWeek(fPreferences->fStartOfWeekOffset);
	}
}


void
MainWindow::_UpdateDayView()
{
	BDate date = _GetSelectedCalendarDate();
	fDayView->SetDate(date);
	LockLooper();
	fDayView->LoadEvents();
	UnlockLooper();
}


BDate
MainWindow::_GetSelectedCalendarDate() const
{
	return fSidePanelView->GetSelectedDate();
}
