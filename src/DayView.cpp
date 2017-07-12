/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "DayView.h"

#include <LayoutBuilder.h>
#include <TimeFormat.h>


DayView::DayView(const BDate& date, BList* eventList)
	:
	BView("DayView", B_WILL_DRAW)
{
	fDate = date;
	fEventList = eventList;

	fEventListView = new BListView("EventList", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW);

	fEventScroll = new BScrollView("EventScroll", fEventListView,
		B_WILL_DRAW, false, true);
	//fEventScroll->SetExplicitMinSize(BSize(260, 260));

	fEventListView->SetInvocationMessage(new BMessage(kInvokationMessage));

	BFont font;
	fEventListView->GetFont(&font);
	font.SetSize(font.Size() + 4);
	fEventListView->SetFont(&font);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fEventScroll)
	.End();


	CheckForEventThisDay();
	AddDayEvents();

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
	CheckForEventThisDay();
	AddDayEvents();
}


void
DayView::MessageReceived(BMessage* message)
{
	switch (message->what) {

		case kInvokationMessage:
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
	BStringItem* item;
	BString eventString;

	for (int32 i = 0; i < fDayEventList->CountItems(); i++) {
		event = ((Event*)fDayEventList->ItemAt(i));
		eventString = "";
		if (event->IsAllDay()) {
			eventString << "All Day" << " - " << event->GetName();
		}
		else
		{
			eventString << GetLocalisedTimeString(event->GetStartDateTime().Time_t())\
				<<" - " << GetLocalisedTimeString(event->GetEndDateTime().Time_t()) \
				<<" - "<< event->GetName();
		}

		item = new BStringItem(eventString.String());
		fEventListView->AddItem(item);
	}
}


void
DayView::CheckForEventThisDay()
{
	Event* event;
	fDayEventList = new BList();

	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		event = ((Event*)fEventList->ItemAt(i));
		if (event->GetStartDateTime().Date() == fDate)
			fDayEventList->AddItem(event);
	}

	fDayEventList->SortItems((int (*)(const void * , const void *))CompareFunc);
}


int32
DayView::GetIndexOf(Event* event)
{
	Event* e;

	for (int32 i = 0; i < fEventList->CountItems(); i++) {
		e = ((Event*)fEventList->ItemAt(i));
		if (e->GetStartDateTime() == event->GetStartDateTime()
			&& e->GetEndDateTime() == event->GetEndDateTime()
			&& e->IsAllDay() == event->IsAllDay()
			&& BString(e->GetName()) == BString(event->GetName())
			&& BString(e->GetPlace()) == BString(event->GetPlace())
			&& BString(e->GetDescription()) == BString(event->GetDescription()))
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
