/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MainWindow.h"

#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <NodeInfo.h>
#include <ToolBar.h>
#include <vector>

#include "App.h"
#include "Button.h"
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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"

extern int32 NotificationLoop(void* data);
extern int32 ImportICalEvents(void* icalFilePtr);


MainWindow::MainWindow()
	:
	BWindow(((App*)be_app)->GetPreferences()->fMainWindowRect,
		B_TRANSLATE_SYSTEM_NAME("Calendar"), B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fEventWindow(NULL)
{
	SetPulseRate(500000);

	_InitInterface();

	if (((App*)be_app)->GetPreferences()->fMainWindowRect == BRect()) {
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

		case kWeekView:
		case kDayView:
		{
			_ToggleEventViewButton(message->what);
			fDayView->MessageReceived(message);
			break;
		}

		case kAgendaView:
		{
			_ToggleEventViewButton(message->what);
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
			fDayView->MessageReceived(message);
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
		{
			Preferences* preferences = ((App*)be_app)->GetPreferences();
			fSidePanelView->MessageReceived(message);
			fSidePanelView->SetStartOfWeek(preferences->fStartOfWeekOffset);
			_UpdateDayView();
			break;
		}

		case B_REFS_RECEIVED:
		{
			int i = 0;
			entry_ref ref;
			BFile file;
			BNodeInfo info;
			char type[B_FILE_NAME_LENGTH];
			QueryDBManager DBManager;

			while (message->HasRef("refs", i)) {
				message->FindRef("refs", i++, &ref);

				file.SetTo(&ref, B_READ_ONLY);
				info.SetTo(&file);
				info.GetType(type);


				if (BString(type) == BString("application/x-calendar-event"))
					_LaunchEventManager(DBManager.GetEvent(ref));

				else if (BString(type) == BString("text/calendar")) {
					thread_id icalThread = spawn_thread(ImportICalEvents,
						"ICal import thread", B_NORMAL_PRIORITY,
						new BFile(&ref, B_READ_ONLY));
					resume_thread(icalThread);

				} else {
					BMessage msg = BMessage(B_REFS_RECEIVED);
					msg.AddRef("refs", &ref);
					((App*)be_app)->PostMessage(&msg);
				}
			}
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

	fAppMenu = new BMenu(B_TRANSLATE("File"));
	BMenuItem* item = new BMenuItem(B_TRANSLATE("About"), new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	fAppMenu->AddItem(item);
	fAppMenu->AddItem(new BMenuItem(B_TRANSLATE("Preferences"), new BMessage(kMenuAppPref)));
	//
	// Google Calendar support is broken.  It should be replaced with a generic solution to
	// be able to sync with various calendars.  Leaving this here for now to give future
	// developer a place to start.
	//
	//fSyncMenu = new BMenu(B_TRANSLATE("Synchronize"));
	//fSyncMenu->AddItem(new BMenuItem(B_TRANSLATE("Google Calendar"), new BMessage(kMenuSyncGCAL)));
	//fAppMenu->AddItem(fSyncMenu);
	fAppMenu->AddSeparatorItem();
	fAppMenu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(kMenuAppQuit), 'Q', B_COMMAND_KEY));

	fEventMenu = new BMenu(B_TRANSLATE("Event"));
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Add event"), new BMessage(kAddEvent)));
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Edit event"), new BMessage(kMenuEventEdit)));
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Remove event"), new BMessage(kMenuEventDelete)));

	fCategoryMenu = new BMenu(B_TRANSLATE("Category"));
	fCategoryMenu->AddItem(new BMenuItem(B_TRANSLATE("Manage categories" B_UTF8_ELLIPSIS),
		new BMessage(kMenuCategoryEdit)));
	fViewMenu = new BMenu(B_TRANSLATE("View"));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Day view"), new BMessage(kDayView)));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Week view"), new BMessage(kWeekView)));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Agenda view"), new BMessage(kAgendaView)));
	fViewMenu->AddSeparatorItem();
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Go to today"), new BMessage(kSetCalendarToCurrentDate)));

	fMenuBar->AddItem(fAppMenu);
	fMenuBar->AddItem(fEventMenu);
	fMenuBar->AddItem(fCategoryMenu);
	fMenuBar->AddItem(fViewMenu);

	fToolBar = new BToolBar();
	fToolBar->AddAction(new BMessage(kSetCalendarToCurrentDate), this, LoadVectorIcon("CALENDAR_ICON"),
		"Today", B_TRANSLATE("Today"), true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kDayView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Day", B_TRANSLATE("Day"), true);
	fToolBar->AddAction(new BMessage(kWeekView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Week", B_TRANSLATE("Week"), true);
	fToolBar->AddAction(new BMessage(kAgendaView), this, LoadVectorIcon("ADD_EVENT"),
		"Agenda", B_TRANSLATE("Agenda"), true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kAddEvent), this, LoadVectorIcon("ADD_EVENT"),
		"Add event", B_TRANSLATE("Add event"), true);
	fToolBar->AddGlue();

	fSidePanelView = new SidePanelView();
	fDayView = new DayView(BDate::CurrentDate(B_LOCAL_TIME));

	_ToggleEventViewButton(kDayView);

	fMainView->StartWatchingAll(fSidePanelView);

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, 0.0f)
		.Add(fDayView)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.Add(fMenuBar)
		.Add(fToolBar)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fSidePanelView, 1)
			.Add(fMainView, 5)
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
	Preferences* preferences = ((App*)be_app)->GetPreferences();

	if (preferences != NULL) {
		if(preferences->fHeaderVisible == true)
			fSidePanelView->ShowWeekHeader(true);
		else
			fSidePanelView->ShowWeekHeader(false);

		fSidePanelView->SetStartOfWeek(preferences->fStartOfWeekOffset);
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


void
MainWindow::_ToggleEventViewButton(int selectedButtonId)
{
	static const std::vector<int> skEventViewButtonIds = { kDayView, kWeekView,
			kAgendaView };

	for (int buttonName : skEventViewButtonIds) {
		BButton* button = fToolBar->FindButton(buttonName);
		BMessage* message = button->Message();
		if (message != NULL) {
			button->SetValue(message->what == (uint32)selectedButtonId);
		}

		BMenuItem* item = fViewMenu->FindItem(buttonName);
		message = button->Message();
		if (message != NULL) {
			item->SetMarked(message->what == (uint32)selectedButtonId);
		}
	}
}
