/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventListView.h"

#include <Catalog.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>

#include "App.h"
#include "Event.h"
#include "EventListItem.h"
#include "EventTabView.h"
#include "MainWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EventListView"

class PopUpMenu : public BPopUpMenu
{
public:
	PopUpMenu(const char* name, BMessenger target);
	virtual ~PopUpMenu();

private:
	BMessenger fTarget;
};


PopUpMenu::PopUpMenu(const char* name, BMessenger target)
	:
	BPopUpMenu(name, false, false),
	fTarget(target)
{
	SetAsyncAutoDestruct(true);
}


PopUpMenu::~PopUpMenu()
{
	fTarget.SendMessage(kPopClosed);
}


EventListView::EventListView(const char* name)
	:
	BListView(name),
	fShowingPopUpMenu(false),
	fPopUpMenuEnabled(true),
	fPrimaryButton(false)
{
}


EventListView::~EventListView()
{
}


void
EventListView::Draw(BRect rect)
{
	float width, height;
	BFont font;

	if (IsEmpty()) {
		SetLowColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		FillRect(rect);
		BString string;
		font.SetSize(font.Size() + 4);
		SetFont(&font);
		string = B_TRANSLATE("You don't have any events on this day!");
		float strwidth = font.StringWidth(string);
		GetPreferredSize(&width, &height);
		MovePenTo(width / 2 - strwidth / 2, height / 2 + font.Size() / 2);
		SetHighColor(ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));
		DrawString(string.String());

	} else {
		SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		BRect bounds(Bounds());
		BRect itemFrame = ItemFrame(CountItems() - 1);
		bounds.top = itemFrame.bottom;
		FillRect(bounds);
	}
	BListView::Draw(rect);
}


void
EventListView::FrameResized(float w, float h)
{
	BListView::FrameResized(w, h);

	for (int32 i = 0; i < CountItems(); i++) {
		BListItem* item = ItemAt(i);
		item->Update(this, be_plain_font);
	}
	Invalidate();
}


void
EventListView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kPopClosed:
		{
			fShowingPopUpMenu = false;
			break;
		}

		case kEditActionInvoked:
		{
			fShowingPopUpMenu = false;
			BView* view = Window()->FindView("EventsView");
			BMessenger msgr(view);
			BMessage msg(kEditEventMessage);
			msgr.SendMessage(&msg);
			break;
		}

		case kDeleteActionInvoked:
		case kCancelActionInvoked:
		case kHideActionInvoked:
		{
			fShowingPopUpMenu = false;
			BView* view = Window()->FindView("EventsView");
			BMessenger msgr(view);
			BMessage msg(kDeleteEventMessage);
			if (message->what == kCancelActionInvoked)
				msg.what = kCancelEventMessage;
			else if (message->what == kHideActionInvoked)
				msg.what = kHideEventMessage;
			msgr.SendMessage(&msg);
			break;
		}

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
EventListView::MouseDown(BPoint position)
{
	uint32 buttons = 0;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		buttons = Window()->CurrentMessage()->FindInt32("buttons");

	if (buttons == B_SECONDARY_MOUSE_BUTTON) {
		Select(IndexOf(position));
		_ShowPopUpMenu(ConvertToScreen(position));
		fPrimaryButton = false;
		Invalidate();
		return;
	}

	if (buttons == B_PRIMARY_MOUSE_BUTTON) {
		fCurrentItemIndex = IndexOf(position);
		fPrimaryButton = true;
	}

	BListView::MouseDown(position);
}


void
EventListView::MouseUp(BPoint position)
{
	if ((fCurrentItemIndex == IndexOf(position) && fPrimaryButton == true)) {
		fPrimaryButton = false;
		fCurrentItemIndex = -1;
	}

	Invalidate();

	BListView::MouseUp(position);
}


void
EventListView::SelectionChanged()
{
	BMessage msg(kEventSelected);
	msg.AddInt32("index", CurrentSelection());

	if (CurrentSelection() > -1) {
		EventListItem* sItem
			= dynamic_cast<EventListItem*>(ItemAt(CurrentSelection()));
		if (sItem != NULL) {
			uint16 status = sItem->GetEvent()->GetStatus();
			msg.AddBool("_cancelled", (status & EVENT_CANCELLED));
			msg.AddBool("_deleted", (status & EVENT_DELETED));
			msg.AddBool("_hidden", (status & EVENT_HIDDEN));
		}
	}
	Messenger().SendMessage(&msg);
}


void
EventListView::MakeEmpty()
{
	BMessage msg(kEventSelected);
	msg.AddInt32("index", -1);
	Messenger().SendMessage(&msg);

	BListView::MakeEmpty();
}


Event*
EventListView::SelectedEvent()
{
	Event* event = NULL;
	if (CurrentSelection() > -1) {
		EventListItem* sItem
			= dynamic_cast<EventListItem*>(ItemAt(CurrentSelection()));
		if (sItem != NULL && sItem->GetEvent() != NULL)
			event = sItem->GetEvent();
	}
	return event;
}


void
EventListView::SetPopUpMenuEnabled(bool enable)
{
	fPopUpMenuEnabled = enable;
}


void
EventListView::_ShowPopUpMenu(BPoint screen)
{
	if (fShowingPopUpMenu == true)
		return;

	EventListItem* sItem
		= dynamic_cast<EventListItem*>(ItemAt(CurrentSelection()));

	if (CurrentSelection() < 0 || sItem == NULL) {
		_ShowEmptyPopUpMenu(screen);
		return;
	}

	uint16 eventStatus = sItem->GetEvent()->GetStatus();

	PopUpMenu* menu = new PopUpMenu("PopUpMenu", this);

	BMenuItem* item;
	item = new BMenuItem(B_TRANSLATE("New" B_UTF8_ELLIPSIS),
		new BMessage(kAddEventMessage), 'N');
	menu->AddItem(item);
	item = new BMenuItem(B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		new BMessage(kEditActionInvoked), 'E');
	menu->AddItem(item);
	item = new BMenuItem(
		B_TRANSLATE("Delete"), new BMessage(kDeleteActionInvoked), 'T');
	item->SetMarked(eventStatus & EVENT_DELETED);
	menu->AddItem(item);
	item = new BMenuItem(
		B_TRANSLATE("Cancel"), new BMessage(kCancelActionInvoked));
	item->SetMarked(eventStatus & EVENT_CANCELLED);
	menu->AddItem(item);
	item = new BMenuItem(B_TRANSLATE("Hide"), new BMessage(kHideActionInvoked));
	item->SetMarked(eventStatus & EVENT_HIDDEN);
	menu->AddItem(item);

	if (fPopUpMenuEnabled == false)
		menu->SetEnabled(false);

	menu->SetTargetForItems(this);
	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}


void
EventListView::_ShowEmptyPopUpMenu(BPoint screen)
{
	PopUpMenu* menu = new PopUpMenu("PopUpMenu", this);

	BMenuItem* item = new BMenuItem(B_TRANSLATE("New event" B_UTF8_ELLIPSIS),
		new BMessage(kAddEventMessage), 'N');
	item->SetTarget(((App*) be_app)->mainWindow());
	menu->AddItem(item);

	if (!fPopUpMenuEnabled)
		menu->SetEnabled(false);

	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}
