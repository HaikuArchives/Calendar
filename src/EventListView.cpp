/*
 * Copyright 2017 Akshay Agarwal<agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "EventListView.h"

#include "EventListItem.h"
#include "MainWindow.h"


class PopUpMenu : public BPopUpMenu {
public:
				PopUpMenu(const char* name, BMessenger target);
	virtual 	~PopUpMenu();

private:
	BMessenger 	fTarget;
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


EventListView::EventListView()
	:
	BListView("EventList"),
	fShowingPopUpMenu(false),
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
		string = "You don't have any events on this day";
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
		cout<<"\n"<<"test3"<<"\n";
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

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
EventListView::MouseDown(BPoint position)
{
	BRect bounds(Bounds());
	BRect itemFrame = ItemFrame(CountItems() - 1);
	bounds.top = itemFrame.bottom;
	if (bounds.Contains(position))
		return;

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
	if ((fCurrentItemIndex == IndexOf(position)
			&& fPrimaryButton == true)) {
			fPrimaryButton = false;
			fCurrentItemIndex = -1;
	}

	Invalidate();

	BListView::MouseUp(position);
}


void
EventListView::_ShowPopUpMenu(BPoint screen)
{
	if (fShowingPopUpMenu || IsEmpty())
		return;

	EventListItem* sItem = dynamic_cast<EventListItem *>
		(ItemAt(CurrentSelection()));

	PopUpMenu* menu = new PopUpMenu("PopUpMenu", this);

	BMenuItem* item;
	item = new BMenuItem("Modify",
			new BMessage(kEventModify), 'E');
	menu->AddItem(item);
	item = new BMenuItem("Delete",
			new BMessage(kEventDelete), 'D');
	menu->AddItem(item);

	menu->SetTargetForItems(this);
	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}
