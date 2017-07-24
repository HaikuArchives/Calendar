/*
 * Copyright 2017 Akshay Agarwal<agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Application.h>
#include <ControlLook.h>

#include "EventListItem.h"


const rgb_color EventListItem::fDefaultColor = {86, 86, 197};


EventListItem::EventListItem(BString name, BString timeText)
	:
	BListItem()
{
	fName = name;
	fTimeText = timeText;
}


EventListItem::~EventListItem()
{
}


void
EventListItem::DrawItem(BView* view, BRect rect, bool complete)
{
	float spacing = be_control_look->DefaultLabelSpacing();
	float offset = spacing;
	BFont namefont;
	BFont timefont;
	font_height finfo;

	// set background color

	rgb_color bgColor;
	if (IsSelected())
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);


	view->SetHighColor(bgColor);
	view->SetLowColor(bgColor);
	view->FillRect(rect);

	// event category

	BRect categoryRect(view->ConvertFromParent(BPoint(rect.left
		+ spacing + 2, rect.top + (rect.Height() -
		fItemHeight/2) / 2)), BSize(fItemHeight/2, fItemHeight/2));

	view->SetHighColor(ui_color(B_CONTROL_BORDER_COLOR));
	view->StrokeRect(categoryRect);
	view->SetHighColor(fDefaultColor);
		//Currently assign all events a default category(color)
	view->FillRect(categoryRect);
	offset = categoryRect.Width() + offset + spacing + 2;

	// event time period

	if (IsSelected())
		view->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		view->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));

	timefont.SetSize(timefont.Size() + 4);
	timefont.GetHeight(&finfo);
	view->SetFont(&timefont);

	view->MovePenTo(offset, rect.top + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2)
		+ (finfo.ascent + finfo.descent) - timefont.Size() + 2 + 3);

	float width, height;
	view->GetPreferredSize(&width, &height);
	view->TruncateString(&fTimeText, B_TRUNCATE_MIDDLE, width - fItemHeight
			- offset / 2);
	view->DrawString(fTimeText.String());

	// event name

	if (IsSelected())
		view->SetHighColor(tint_color(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR),
			0.7));
	else
		view->SetHighColor(tint_color(ui_color(B_LIST_ITEM_TEXT_COLOR), 0.6));

	namefont.SetSize(timefont.Size() - 2);
	namefont.GetHeight(&finfo);
	view->SetFont(&namefont);

	view->MovePenTo(offset,
		rect.top + timefont.Size() - namefont.Size() + 4 + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2)
		+ (finfo.ascent + finfo.descent));

		view->TruncateString(&fName, B_TRUNCATE_MIDDLE, width - fItemHeight
			- offset / 2);
		view->DrawString(fName.String());

	// draw lines

	view->SetHighColor(tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR),
		B_DARKEN_1_TINT));
	view->StrokeLine(rect.LeftBottom(), rect.RightBottom());
}


void
EventListItem::Update(BView* owner, const BFont* finfo)
{
	// we need to override the update method so we can make sure the
	// list item size doesn't change
	BListItem::Update(owner, finfo);

	float spacing = be_control_look->DefaultLabelSpacing();
	SetHeight(fItemHeight + spacing + 4);
}
