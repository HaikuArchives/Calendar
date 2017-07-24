/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MainWindow.h"
#include <Application.h>
#include <DateFormat.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>

#include "MainView.h"
#include "PreferenceWindow.h"
#include "ResourceLoader.h"
#include "SidePanelView.h"

#include "EventListView.h"


MainWindow::MainWindow()
	:
	BWindow(BRect(), "Calendar", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fEventWindow(NULL),
	fPreferences(NULL)
{
	SetPulseRate(500000);

	ResizeTo(640, 360);
	CenterOnScreen();

	fMainView = new MainView();

	fMenuBar = new BMenuBar("MenuBar");

	fAppMenu = new BMenu("App");
	BMenuItem* item = new BMenuItem("About", new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	fAppMenu->AddItem(item);
	fAppMenu->AddSeparatorItem();
	fAppMenu->AddItem(new BMenuItem("Quit", new BMessage(kMenuAppQuit), 'Q', B_COMMAND_KEY));

	fEditMenu = new BMenu("Edit");
	fEditMenu->AddItem(new BMenuItem("Preferences", new BMessage(kMenuEditPref)));
	fEditMenu->AddSeparatorItem();
	fEditMenu->AddItem(new BMenuItem("Category", new BMessage(kMenuEditCategory)));

	fMenuBar->AddItem(fAppMenu);
	fMenuBar->AddItem(fEditMenu);

	fToolBar = new BToolBar();
	fToolBar->AddAction(new BMessage(kSetCalendarToCurrentDate), this, LoadVectorIcon("CALENDAR_ICON"),
		"Today", "Today", true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kDayView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Day View", "Day View", true);
	fToolBar->AddAction(new BMessage(kMonthView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Month View", "Month View", true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kAddEvent), this, LoadVectorIcon("ADD_EVENT"),
		"Add Event", "Add Event", true);
	fToolBar->AddGlue();

	fEventList = new BList();
	fSidePanelView = new SidePanelView();
	fDayView = new DayView(BDate::CurrentDate(B_LOCAL_TIME), fEventList);

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
			_LaunchEventManager(-1);
			break;

		case kModifyEventMessage:
		{	int32 index;
			message->FindInt32("index", &index);
			_LaunchEventManager(index);
			break;
		}

		case kEventWindowQuitting:
		{
			fEventWindow = NULL;
			_UpdateDayView();
			break;
		}

		case kSetCalendarToCurrentDate:
		{
			fSidePanelView->MessageReceived(message);
			break;
		}

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
		{
			fSidePanelView->MessageReceived(message);
			break;
		}

		case B_LOCALE_CHANGED:
			fSidePanelView->MessageReceived(message);
			break;

		case kAppPreferencesChanged:
			_SyncWithPreferences();
			break;

		case kMenuEditPref:
			be_app->PostMessage(message);
			break;

		case kMenuEditCategory:
			be_app->PostMessage(message);
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
MainWindow::_LaunchEventManager(int32 index)
{
	if (fEventWindow == NULL) {
		if (index == -1) {
			Event* event = NULL;
			fEventWindow = new EventWindow();
			fEventWindow->SetEvent(event, index, fEventList);
			BDate date = _GetSelectedCalendarDate();
			fEventWindow->SetEventDate(date);
		}

		else
		{
			Event* event = ((Event*)fEventList->ItemAt(index));
			fEventWindow = new EventWindow();
			fEventWindow->SetEvent(event, index, fEventList);
		}

		fEventWindow->Show();
	}

	fEventWindow->Activate();

}


void
MainWindow::SetPreferences(Preferences* preferences)
{
	fPreferences = preferences;
	_SyncWithPreferences();
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
	LockLooper();
	BDate date = _GetSelectedCalendarDate();
	fDayView->Update(date, fEventList);
	UnlockLooper();
}


BDate
MainWindow::_GetSelectedCalendarDate() const
{
	return fSidePanelView->GetSelectedDate();
}
