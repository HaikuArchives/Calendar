/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "DayView.h"

#include <time.h>

#include <Alert.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <List.h>
#include <ScrollView.h>

#include "Event.h"
#include "EventListItem.h"
#include "EventListView.h"
#include "QueryDBManager.h"
#include "SidePanelView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DayView"

DayView::DayView(const BDate& date)
	:
	BView("DayView", B_WILL_DRAW)
{
	fDate = date;
	fMode = kDayView; // start with Day View
	fEventListView = new EventListView();
	fEventListView->SetViewColor(B_TRANSPARENT_COLOR);
	fEventListView->SetInvocationMessage(new BMessage(kInvokationMessage));
	fEventList = new BList();

	fEventScroll = new BScrollView("EventScroll", fEventListView,
		B_WILL_DRAW, false, true);
	fEventScroll->SetExplicitMinSize(BSize(260, 260));

	fDBManager = new QueryDBManager();
	LoadEvents();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fEventScroll)
	.End();
}


void
DayView::AttachedToWindow()
{
	fEventListView->SetTarget(this);
}


void
DayView::SetDate(const BDate& date)
{
	fDate = date;
}

void
DayView::LoadEvents()
{
	if (!fEventList->IsEmpty()) {
		fEventList->MakeEmpty();
		fEventListView->MakeEmpty();
	}

	if (fMode == kWeekView)
		fEventList = fDBManager->GetEventsOfWeek(fDate);
	else
		fEventList = fDBManager->GetEventsOfDay(fDate); // Day and Agenda views

	fEventList->SortItems((int (*)(const void *, const void *))CompareFunc);
	_PopulateEvents();
	fEventListView->Invalidate();
}


void
DayView::MessageReceived(BMessage* message)
{
	switch (message->what) {

		case kInvokationMessage:
		case kEditEventMessage:
		{
			int32 selection = fEventListView->CurrentSelection();
			if (selection >= 0) {
				Event* event = ((Event*)fEventList->ItemAt(selection));
				BMessage msg(kLaunchEventManagerToModify);
				msg.AddPointer("event", event);
				Window()->PostMessage(&msg);
			}
			break;
		}
		case kDeleteEventMessage:
		case kCancelEventMessage:
		{
			int32 selection = fEventListView->CurrentSelection();
			if (selection < 0)
				return;
			Event* event = ((Event*)fEventList->ItemAt(selection));
			bool isCancelled = (event->GetStatus() & EVENT_CANCELLED);

			BString title(B_TRANSLATE("Confirm delete"));
			BString label(B_TRANSLATE("Are you sure you want to move the selected event to Trash?"));
			if (message->what == kCancelEventMessage) {
				title = B_TRANSLATE("Confirm cancellation");
				label = B_TRANSLATE("Are you sure you want to cancel the selected event?");
			}

			// If disabling a previous cancellation, the confirmation dialogue
			// doesn't really make sense.
			int32 button_index = 0;
			if (!(message->what == kCancelEventMessage && isCancelled == true)) {
				BAlert* alert = new BAlert(title, label, NULL, B_TRANSLATE("OK"),
					B_TRANSLATE("Cancel"), B_WIDTH_AS_USUAL, B_WARNING_ALERT);
				alert->SetShortcut(1, B_ESCAPE);
				button_index = alert->Go();
			}

			if (button_index == 0) {
				Event newEvent(*event);
				newEvent.SetUpdated(time(NULL));
				if (message->what == kCancelEventMessage && isCancelled == false)
					newEvent.SetStatus(newEvent.GetStatus() | EVENT_CANCELLED);
				else if (message->what == kCancelEventMessage)
					newEvent.SetStatus(newEvent.GetStatus() & ~EVENT_CANCELLED);
				else
					newEvent.SetStatus(newEvent.GetStatus() | EVENT_DELETED);

				fDBManager->UpdateEvent(event, &newEvent);
				Window()->LockLooper();
				LoadEvents();
				Window()->UnlockLooper();
			}
			break;
		}
		case kDayView:
		case kWeekView:
		case kAgendaView:
			fMode = message->what;

		case kSetCalendarToCurrentDate:
			LoadEvents();
			break;

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
DayView::SetEventListPopUpEnabled(bool state)
{
	fEventListView->SetPopUpMenuEnabled(state);
}


int
DayView::CompareFunc(const void* a, const void* b)
{

	if ((*(Event**) a)->IsAllDay() && !(*(Event**) b)->IsAllDay())
		return -1;
	else if ((*(Event**) b)->IsAllDay() && !(*(Event**) a)->IsAllDay())
		return 1;
	else if (difftime((*(Event**) a)->GetStartDateTime(), (*(Event**) b)->GetStartDateTime()) < 0 )
		return -1;
	else if (difftime((*(Event**) a)->GetStartDateTime(), (*(Event**) b)->GetStartDateTime()) > 0)
		return 1;
	else
		return 0;
}


void
DayView::_PopulateEvents()
{
	Event* event;
	EventListItem* item;

	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		event = ((Event*)fEventList->ItemAt(i));
		item = new EventListItem(event, fMode);
		fEventListView->AddItem(item);
	}
}
