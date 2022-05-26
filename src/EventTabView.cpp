/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2021, Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventTabView.h"

#include <Alert.h>
#include <Catalog.h>
#include <ScrollView.h>
#include <Window.h>

#include "App.h"
#include "EventListItem.h"
#include "EventListView.h"
#include "QueryDBManager.h"
#include "SidePanelView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DayView"


EventTabView::EventTabView(const BDate& date)
	:
	BTabView("EventsView")
{
	_AddEventList("Day", B_TRANSLATE("Day"), kDayTab);
	_AddEventList("Week", B_TRANSLATE("Week"), kWeekTab);
	_AddEventList("Month", B_TRANSLATE("Month"), kMonthTab);

	fMode = ((App*) be_app)->GetPreferences()->fViewMode;
	fPopUpEnabled = true;
	fEventList = NULL;
	fDBManager = new QueryDBManager();
	SetDate(date);
}


EventTabView::~EventTabView()
{
	delete fEventList;
	delete fDBManager;
}


void
EventTabView::AttachedToWindow()
{
	Select(((App*) be_app)->GetPreferences()->fSelectedTab);
	Window()->MessageReceived(new BMessage(kListModeChanged));

	EventListView* list = ListAt(Selection());
	if (list != NULL)
		list->SetTarget(this);
}


void
EventTabView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kInvokationMessage:
		case kEditEventMessage:
		{
			Event* event = SelectedEvent();
			if (event != NULL) {
				BMessage msg(kLaunchEventManagerToModify);
				msg.AddPointer("event", event);
				Window()->PostMessage(&msg);
			}
			break;
		}
		case kDeleteEventMessage:
		case kCancelEventMessage:
		case kHideEventMessage:
		{
			Event* event = SelectedEvent();
			if (event == NULL)
				return;
			bool isCancelled = (event->GetStatus() & EVENT_CANCELLED);
			bool isHidden = (event->GetStatus() & EVENT_HIDDEN);
			bool isDeleted = (event->GetStatus() & EVENT_DELETED);

			// If deleting an event, make sure user's prepared for the
			// dire consequences!
			int32 button_index = 0;
			if (message->what == kDeleteEventMessage && isDeleted == false) {
				BAlert* alert = new BAlert(B_TRANSLATE("Confirm delete"),
					B_TRANSLATE("Are you sure you want to move the selected "
								"event to Trash?"),
					NULL, B_TRANSLATE("OK"), B_TRANSLATE("Cancel"),
					B_WIDTH_AS_USUAL, B_WARNING_ALERT);
				alert->SetShortcut(1, B_ESCAPE);
				button_index = alert->Go();
			}

			if (button_index == 0) {
				Event newEvent(*event);
				newEvent.SetUpdated(time(NULL));
				if (message->what == kCancelEventMessage
					&& isCancelled == false)
					newEvent.SetStatus(newEvent.GetStatus() | EVENT_CANCELLED);
				else if (message->what == kCancelEventMessage)
					newEvent.SetStatus(newEvent.GetStatus() & ~EVENT_CANCELLED);
				else if (message->what == kHideEventMessage
					&& isHidden == false)
					newEvent.SetStatus(newEvent.GetStatus() | EVENT_HIDDEN);
				else if (message->what == kHideEventMessage)
					newEvent.SetStatus(newEvent.GetStatus() & ~EVENT_HIDDEN);
				else if (message->what == kDeleteEventMessage
					&& isDeleted == false)
					newEvent.SetStatus(newEvent.GetStatus() | EVENT_DELETED);
				else if (message->what == kDeleteEventMessage
					&& isDeleted == true)
					newEvent.SetStatus(newEvent.GetStatus() & ~EVENT_DELETED);

				fDBManager->UpdateEvent(event, &newEvent);
				Window()->LockLooper();
				LoadEvents();
				Window()->UnlockLooper();
			}

			if ((message->what == kDeleteEventMessage
					|| message->what == kHideEventMessage)
				&& ((App*) be_app)->GetPreferences()->fFirstDeletion == true) {
				((App*) be_app)->GetPreferences()->fFirstDeletion = false;
				BAlert* alert = new BAlert(B_TRANSLATE("Managing events"),
					B_TRANSLATE("Keep in mind, you can manage deleted and "
								"hidden events by toggling "
								"\"Show hidden/deleted events\" in the \"View\" menu."),
					B_TRANSLATE("OK"));
				alert->Go();
			}
			break;
		}
		case kChangeListMode:
		{
			uint8 mode = 0;
			if (message->FindUInt8("mode", &mode) == B_OK)
				ToggleMode(mode);
			break;
		}
		case kChangeListTab:
		{
			int32 tab = message->GetInt32("tab", -1);
			Select(tab);
			break;
		}
		case kSetCalendarToCurrentDate:
			LoadEvents();
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}


void
EventTabView::Select(int32 index)
{
	if (index < 0)
		return;
	if (index == kDayTab)
		fMode &= ~kDateView;
	else
		fMode |= kDateView;
	BTabView::Select(index);
	ListAt(index)->SetPopUpMenuEnabled(fPopUpEnabled);
	LoadEvents();

	((App*) be_app)->GetPreferences()->fSelectedTab = index;
	((App*) be_app)->GetPreferences()->fViewMode = fMode;
	Window()->MessageReceived(new BMessage(kListTabChanged));
}


void
EventTabView::SetDate(const BDate& date)
{
	fDate = date;
	LoadEvents();
}


void
EventTabView::ToggleMode(uint8 flag)
{
	fMode ^= flag;
	_PopulateList();

	((App*) be_app)->GetPreferences()->fViewMode = fMode;
	Window()->MessageReceived(new BMessage(kListModeChanged));
}


uint8
EventTabView::Mode()
{
	return fMode;
}


void
EventTabView::SetPopUpEnabled(bool state)
{
	fPopUpEnabled = state;
}


Event*
EventTabView::SelectedEvent()
{
	EventListView* list = ListAt(Selection());
	Event* event = NULL;
	if (list != NULL)
		event = list->SelectedEvent();
	return event;
}


void
EventTabView::LoadEvents()
{
	delete fEventList;
	switch (Selection()) {
		case kDayTab:
			fEventList = fDBManager->GetEventsOfDay(fDate, false);
			break;
		case kWeekTab:
			fEventList = fDBManager->GetEventsOfWeek(fDate, false);
			break;
		default:
			fEventList = fDBManager->GetEventsOfMonth(fDate, false);
			break;
	}

	fEventList->SortItems(_CompareFunc);
	_PopulateList();
}


EventListView*
EventTabView::ListAt(int32 index)
{
	BTab* tab = TabAt(index);
	EventListView* list = NULL;
	if (tab != NULL) {
		BScrollView* scroll = (BScrollView*) tab->View();
		if (scroll != NULL)
			list = (EventListView*) scroll->Target();
	}
	return list;
}


void
EventTabView::_AddEventList(const char* name, const char* label, int32 tab)
{
	EventListView* listView
		= new EventListView(BString(name).Append("EventList"));
	listView->SetInvocationMessage(new BMessage(kInvokationMessage));

	BScrollView* scrollView = new BScrollView(
		BString(name).Append("Scroll"), listView, B_WILL_DRAW, false, true);
	scrollView->SetExplicitMinSize(BSize(260, 260));
	scrollView->SetBorders(0);

	AddTab(scrollView);
	TabAt(tab)->SetLabel(label);
}


void
EventTabView::_PopulateList()
{
	EventListView* list = ListAt(Selection());
	if (list == NULL)
		return;

	list->MakeEmpty();
	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		Event* event = fEventList->ItemAt(i);
		if (event == NULL)
			continue;

		bool hidden = (fMode & kHiddenView);
		uint16 eventStatus = event->GetStatus();
		if (hidden == false
			&& ((eventStatus & EVENT_DELETED) || (eventStatus & EVENT_HIDDEN)))
			continue;

		EventListItem* item = new EventListItem(event, fMode);
		list->AddItem(item);
	}
	list->Invalidate();
}


int
EventTabView::_CompareFunc(const Event* a, const Event* b)
{
	if (a->IsAllDay() == true && b->IsAllDay() == false)
		return -1;
	else if (b->IsAllDay() == true && a->IsAllDay())
		return 1;
	else if (difftime(a->GetStartDateTime(), b->GetStartDateTime()) < 0)
		return -1;
	else if (difftime(a->GetStartDateTime(), b->GetStartDateTime()) > 0)
		return 1;
	else
		return 0;
}
