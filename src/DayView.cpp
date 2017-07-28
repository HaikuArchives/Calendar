/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "DayView.h"

#include <LayoutBuilder.h>
#include <TimeFormat.h>

#include "EventListItem.h"


DayView::DayView(const BDate& date, BList* eventList)
	:
	BView("DayView", B_WILL_DRAW)
{
	fDate = date;
	fEventList = eventList;

	fEventListView = new EventListView();
	fEventListView->SetViewColor(B_TRANSPARENT_COLOR);
	//fEventListView->SetInvocationMessage(new BMessage(kInvokationMessage));

	fEventScroll = new BScrollView("EventScroll", fEventListView,
		B_WILL_DRAW, false, true);
	fEventScroll->SetExplicitMinSize(BSize(260, 260));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fEventScroll)
	.End();

	if (CheckForEventThisDay()) {
		SortEvents();
		AddDayEvents();
	}
}


void
DayView::AttachedToWindow()
{
	fEventListView->SetTarget(this);
}


void
DayView::Update(const BDate& date, BList* eventList)
{
	fDate = date;
	fEventList = eventList;

	fDayEventList->MakeEmpty();
	fEventListView->MakeEmpty();

	if (CheckForEventThisDay()) {
		SortEvents();
		AddDayEvents();
	}

	fEventListView->Invalidate();
}


void
DayView::MessageReceived(BMessage* message)
{
	switch (message->what) {

		case kEventModify:
		{
			int32 selection = fEventListView->CurrentSelection();
			if (selection >= 0) {
				Event* event = ((Event*)fDayEventList->ItemAt(selection));
				int32 eventIndex = GetIndexOf(event);
				BMessage msg(kModifyEventMessage);
				msg.AddInt32("index", eventIndex);
				Window()->PostMessage(&msg);
			}
			break;
		}

		case kEventDelete:
		{
			int32 selection = fEventListView->CurrentSelection();
			if (selection >= 0) {
				Event* event = ((Event*)fDayEventList->ItemAt(selection));
				int32 eventIndex = GetIndexOf(event);
				fEventList->RemoveItem(eventIndex);
				Window()->LockLooper();
				Update(fDate, fEventList);
				Window()->UnlockLooper();
			}
			break;
		}

		default:
			BView::MessageReceived(message);
			break;
	}
}


int
DayView::CompareFunc(const void* a, const void* b)
{

	if ((*(Event**) a)->IsAllDay() && !(*(Event**) b)->IsAllDay())
		return -1;
	else if ((*(Event**) b)->IsAllDay() && !(*(Event**) a)->IsAllDay())
		return 1;
	else if ((*(Event**) a)->GetStartDateTime() < (*(Event**) b)->GetStartDateTime())
		return -1;
	else if ((*(Event**) a)->GetStartDateTime() > (*(Event**) b)->GetStartDateTime())
		return 1;
	else
		return 0;
}


void
DayView::AddDayEvents()
{
	Event* event;
	EventListItem* item;
	BString timeString;
	BString nameString;

	for (int32 i = 0; i < fDayEventList->CountItems(); i++) {
		event = ((Event*)fDayEventList->ItemAt(i));
		nameString = "";
		timeString = "";
		if (event->IsAllDay())
			timeString = "All Day";
		else
			timeString << GetLocalisedTimeString(event->GetStartDateTime().Time_t()) \
				<<" - " << GetLocalisedTimeString(event->GetEndDateTime().Time_t());

		nameString << event->GetName();

		item = new EventListItem(nameString, timeString);
		fEventListView->AddItem(item);
	}
}


void
DayView::SortEvents()
{
	fDayEventList->SortItems((int (*)(const void * , const void *))CompareFunc);
}


bool
DayView::CheckForEventThisDay()
{
	Event* event;
	fDayEventList = new BList();

	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		event = ((Event*)fEventList->ItemAt(i));
		if (event->GetStartDateTime().Date() == fDate)
			fDayEventList->AddItem(event);
	}

	if (fDayEventList->IsEmpty())
		return false;

	return true;
}


int32
DayView::GetIndexOf(Event* event)
{
	Event* e;

	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		e = ((Event*)fEventList->ItemAt(i));
		if (e->Equals(*event))
				return i;
	}

	return -1;
}


BString
DayView::GetLocalisedTimeString(time_t time)
{
	BString timeString;
	BTimeFormat().Format(timeString, time,
		B_SHORT_TIME_FORMAT);
	return timeString;
}
