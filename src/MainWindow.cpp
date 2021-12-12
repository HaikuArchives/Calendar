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
#include <vector>

#include "App.h"
#include "Button.h"
#include "CategoryEditWindow.h"
#include "Event.h"
#include "EventListView.h"
#include "EventSyncWindow.h"
#include "EventTabView.h"
#include "EventWindow.h"
#include "MainView.h"
#include "Preferences.h"
#include "PreferenceWindow.h"
#include "ResourceLoader.h"
#include "SidePanelView.h"


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
		case kEditEventMessage:
		case kDeleteEventMessage:
		case kCancelEventMessage:
		case kHideEventMessage:
		case kChangeListMode:
		case kChangeListTab:
			fEventsView->MessageReceived(message);
			break;
		case kListTabChanged:
		{
			for (int i = 0; i < fEventsView->CountTabs(); i++)
				fViewMenu->ItemAt(i)->SetMarked(false);
			fViewMenu->ItemAt(fEventsView->Selection())->SetMarked(true);
			break;
		}
		case kListModeChanged:
		{
			bool agenda = (fEventsView->Mode() & kAgendaView);
			bool hidden = (fEventsView->Mode() & kHiddenView);
			fViewMenu->FindItem(B_TRANSLATE("Agenda mode"))->SetMarked(agenda);
			fViewMenu->FindItem(B_TRANSLATE("Show hidden/deleted events"))->SetMarked(hidden);

			if (hidden == true)
				((App*)be_app)->GetPreferences()->fFirstDeletion = false;

			message->AddBool("hidden", hidden);
			fSidePanelView->MessageReceived(message);
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
			_UpdateEventsView();
			_SetEventListPopUpEnabled(true);
			fEventMenu->SetEnabled(true);
			break;
		}
		case kSetCalendarToCurrentDate:
			fSidePanelView->MessageReceived(message);
			fEventsView->MessageReceived(message);
			break;
		case kSelectedDateChanged:
		case kSynchronizationComplete:
			_UpdateEventsView();
			break;
		case kSelectionMessage:
			fSidePanelView->MessageReceived(message);
			break;
		case kEventSelected:
			_ToggleEventMenu(message);
			break;
		case kMonthUpMessage:
		case kMonthDownMessage:
			fSidePanelView->MessageReceived(message);
			break;
		case B_LOCALE_CHANGED:
		{
			Preferences* preferences = ((App*)be_app)->GetPreferences();
			fSidePanelView->MessageReceived(message);
			fSidePanelView->SetStartOfWeek(preferences->fStartOfWeekOffset);
			_UpdateEventsView();
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
					_LaunchEventManager(NULL, &ref);

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
		case kAppPreferencesChanged:
			_SyncWithPreferences();
			break;
		case kMenuAppPref:
		case kMenuCategoryEdit:
		case kMenuSyncGCAL:
			be_app->PostMessage(message);
			break;
		case kRefreshCategoryList:
		{
			_UpdateEventsView();
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

	fAppMenu = new BMenu(B_TRANSLATE("App"));
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
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Edit event"), new BMessage(kEditEventMessage)));
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Remove event"), new BMessage(kDeleteEventMessage)));
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Cancel event"), new BMessage(kCancelEventMessage)));
	fEventMenu->AddItem(new BMenuItem(B_TRANSLATE("Hide event"), new BMessage(kHideEventMessage)));
	for (int i = 1; i < fEventMenu->CountItems(); i++)
		fEventMenu->ItemAt(i)->SetEnabled(false);

	fCategoryMenu = new BMenu(B_TRANSLATE("Category"));
	fCategoryMenu->AddItem(new BMenuItem(B_TRANSLATE("Manage categories" B_UTF8_ELLIPSIS),
		new BMessage(kMenuCategoryEdit)));

	BMessage* agendaMsg = new BMessage(kChangeListMode);
	agendaMsg->AddUInt8("mode", (uint8)kAgendaView);
	BMessage* hiddenMsg = new BMessage(kChangeListMode);
	hiddenMsg->AddUInt8("mode", (uint8)kHiddenView);

	BMessage* dayMsg = new BMessage(kChangeListTab);
	dayMsg->AddInt32("tab", kDayTab);
	BMessage* weekMsg = new BMessage(kChangeListTab);
	weekMsg->AddInt32("tab", kWeekTab);
	BMessage* monthMsg = new BMessage(kChangeListTab);
	monthMsg->AddInt32("tab", kMonthTab);

	fViewMenu = new BMenu(B_TRANSLATE("View"));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Day view"), dayMsg));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Week view"), weekMsg));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Month view"), monthMsg));
	fViewMenu->AddSeparatorItem();
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Agenda mode"), agendaMsg));
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Show hidden/deleted events"), hiddenMsg));
	fViewMenu->AddSeparatorItem();
	fViewMenu->AddItem(new BMenuItem(B_TRANSLATE("Go to today"), new BMessage(kSetCalendarToCurrentDate)));

	fMenuBar->AddItem(fAppMenu);
	fMenuBar->AddItem(fEventMenu);
	fMenuBar->AddItem(fCategoryMenu);
	fMenuBar->AddItem(fViewMenu);

	fSidePanelView = new SidePanelView();
	fEventsView = new EventTabView(BDate::CurrentDate(B_LOCAL_TIME));

	fMainView->StartWatchingAll(fSidePanelView);

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, 0.0f)
		.SetInsets(0, 0, -2, -2)
		.Add(fEventsView)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.Add(fMenuBar)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fSidePanelView, 1)
			.Add(fMainView, 5)
		.End()
	.End();
}


void
MainWindow::_LaunchEventManager(Event* event, entry_ref* ref)
{
	if (fEventWindow == NULL) {
		fEventWindow = new EventWindow();
		if (event != NULL)
			fEventWindow->SetEvent(event);
		else if (ref != NULL)
			fEventWindow->SetEvent(*ref);

		if (event == NULL && ref == NULL) {
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
	fEventsView->SetPopUpEnabled(state);
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
MainWindow::_UpdateEventsView()
{
	BDate date = _GetSelectedCalendarDate();
	LockLooper();
	fEventsView->SetDate(date);
	UnlockLooper();
}


BDate
MainWindow::_GetSelectedCalendarDate() const
{
	return fSidePanelView->GetSelectedDate();
}


void
MainWindow::_ToggleEventMenu(BMessage* msg)
{
	int32 index = msg->GetInt32("index", -1);
	bool deleted = msg->GetBool("_deleted", false);
	bool cancelled = msg->GetBool("_cancelled", false);
	bool hidden = msg->GetBool("_hidden", false);

	for (int i = 1; i < fEventMenu->CountItems(); i++)
		fEventMenu->ItemAt(i)->SetEnabled(index > -1);
	fEventMenu->ItemAt(2)->SetMarked(deleted);
	fEventMenu->ItemAt(3)->SetMarked(cancelled);
	fEventMenu->ItemAt(4)->SetMarked(hidden);
}
