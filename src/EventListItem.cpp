/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Application.h>
#include <ControlLook.h>
#include <Font.h>
#include <MenuItem.h>

#include "EventListItem.h"


EventListItem::EventListItem(BString name, BString timeText, rgb_color color)
	:
	BListItem()
{
	fName = name;
	fTimeText = timeText;
	fColor = color;
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

	rgb_color bgColor;
	if (IsSelected())
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);

	view->SetHighColor(bgColor);
	view->SetLowColor(bgColor);
	view->FillRect(rect);

	timefont.SetSize(timefont.Size() + 3);
	timefont.GetHeight(&finfo);
	view->SetFont(&timefont);

	// event category indicator

	BRect colorRect(rect);
	colorRect.left += spacing + 2;
	colorRect.right = colorRect.left + rect.Height() / 4;
	colorRect.top = rect.top + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2);
		- timefont.Size();

	colorRect.bottom = colorRect.top + rect.Height() / 4;
	view->SetHighColor(fColor);
	view->FillEllipse(colorRect);
	view->SetHighUIColor(B_CONTROL_BORDER_COLOR);
	view->StrokeEllipse(colorRect);
	offset += spacing + colorRect.Width() + 2;

	// event time period
	rgb_color color;
	
	if (IsSelected())
	{
		color = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
		color.alpha = 0.4;
		view->SetHighColor(color);
	} else {
		color = ui_color(B_LIST_ITEM_TEXT_COLOR);
		color.alpha = 0.4;
		view->SetHighColor(color);
	}

	view->MovePenTo(offset,
		rect.top + timefont.Size() - namefont.Size() + 6 + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2)
		+ (finfo.ascent + finfo.descent));
		
	BString timeText(fTimeText);
	view->TruncateString(&timeText, B_TRUNCATE_END,  rect.Width() - offset - 2);
	view->DrawString(timeText.String());

	// event name
	
	if (IsSelected())
	{
		color = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
		color.alpha = 0.6;
		view->SetHighColor(color);
	} else {
		color = ui_color(B_LIST_ITEM_TEXT_COLOR);
		color.alpha = 0.6;
		view->SetHighColor(color);
	}

	namefont.SetSize(timefont.Size() + 2);
	namefont.GetHeight(&finfo);
	view->SetFont(&namefont);
	
	view->MovePenTo(offset, rect.top + ((rect.Height()
		- (finfo.ascent + finfo.descent + finfo.leading)) / 2)
		+ (finfo.ascent + finfo.descent) - timefont.Size() + 2);
		
	BString name(fName);
	view->TruncateString(&name, B_TRUNCATE_END, rect.Width() - offset - 2);
	view->DrawString(name.String());

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
	SetHeight(fItemHeight + spacing * 2);
}
