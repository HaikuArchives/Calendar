/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "DayView.h"

#include <time.h>

#include <Alert.h>
#include <LayoutBuilder.h>
#include <List.h>
#include <ScrollView.h>
#include <TimeFormat.h>

#include "Event.h"
#include "EventListItem.h"
#include "EventListView.h"
#include "SQLiteManager.h"


DayView::DayView(const BDate& date)
	:
	BView("DayView", B_WILL_DRAW)
{
	fDate = date;

	fEventListView = new EventListView();
	fEventListView->SetViewColor(B_TRANSPARENT_COLOR);
	fEventListView->SetInvocationMessage(new BMessage(kInvokationMessage));
	fEventList = new BList();

	fEventScroll = new BScrollView("EventScroll", fEventListView,
		B_WILL_DRAW, false, true);
	fEventScroll->SetExplicitMinSize(BSize(260, 260));

	fDBManager = new SQLiteManager();
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

	fEventList = fDBManager->GetEventsOfDay(fDate);
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
		{

			int32 selection = fEventListView->CurrentSelection();
			if (selection >= 0) {
				Event* event = ((Event*)fEventList->ItemAt(selection));

				BAlert* alert = new BAlert("Confirm delete",
					"Are you sure you want to delete the selected event?",
					NULL, "OK", "Cancel", B_WIDTH_AS_USUAL, B_WARNING_ALERT);

				alert->SetShortcut(1, B_ESCAPE);
				int32 button_index = alert->Go();

				if (button_index == 0) {
					Event newEvent(*event);
					newEvent.SetStatus(false);
					newEvent.SetUpdated(time(NULL));
					fDBManager->UpdateEvent(event, &newEvent);
					Window()->LockLooper();
					LoadEvents();
					Window()->UnlockLooper();
				}
			}

			break;
		}

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
	BString startTime;
	BString endTime;
	BString eventName;
	BString timePeriod;
	BTimeFormat timeFormat;

	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		event = ((Event*)fEventList->ItemAt(i));

		eventName = "";
		startTime = "";
		endTime = "";
		timePeriod = "";

		if (event->IsAllDay())
			timePeriod = "All day";
		else
		{
			timeFormat.Format(startTime, event->GetStartDateTime(),
				B_SHORT_TIME_FORMAT);
			timeFormat.Format(endTime, event->GetEndDateTime(),
				B_SHORT_TIME_FORMAT);
			timePeriod << startTime << " - " << endTime;
		}

		eventName << event->GetName();
		rgb_color color = event->GetCategory()->GetColor();

		item = new EventListItem(eventName, timePeriod , color);
		fEventListView->AddItem(item);
	}

}
