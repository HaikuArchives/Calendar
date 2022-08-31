/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2010-2017 Humdinger, humdingerb@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventListItem.h"

#include <Application.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <DateFormat.h>
#include <DurationFormat.h>
#include <Font.h>
#include <MenuItem.h>
#include <TimeFormat.h>

#include "ColorConverter.h"
#include "Event.h"
#include "EventTabView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EventListItem"


EventListItem::EventListItem(Event* event, int32 mode)
	:
	fEvent(event),
	BListItem()
{
	_CalcTimeText(mode);
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

	uint16 status = fEvent->GetStatus();
	int severity = 0;
	if (status & EVENT_HIDDEN)
		severity = 1;
	if (status & EVENT_DELETED)
		severity = 2;
	if (IsSelected() == true)
		severity = 0;

	rgb_color bgColor;
	if (IsSelected() == true)
		bgColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		bgColor = ui_color(B_LIST_BACKGROUND_COLOR);
	bgColor = TintColor(bgColor, bgColor, severity);

	if (severity >= 1) {
		rgb_color hideDeleteColor = {255, 0, 0, 255};
		bgColor = mix_color(bgColor, hideDeleteColor, 64);
	}

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
	colorRect.top = rect.top
		+ ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading))
			/ 2);
	-timefont.Size();

	colorRect.bottom = colorRect.top + rect.Height() / 4;
	view->SetHighColor(fEvent->GetCategory()->GetColor());
	view->FillEllipse(colorRect);
	view->SetHighUIColor(B_CONTROL_BORDER_COLOR);
	view->StrokeEllipse(colorRect);
	offset += spacing + colorRect.Width() + 2;

	// event time period
	rgb_color textColor;

	float tint = B_NO_TINT;
	float lightTime = B_LIGHTEN_2_TINT;
	float darkTime = B_DARKEN_3_TINT;

	if (IsSelected())
		textColor = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
	else
		textColor = ui_color(B_LIST_ITEM_TEXT_COLOR);
	view->SetHighColor(TintColor(textColor, textColor, 1));

	view->MovePenTo(offset,
		rect.top + timefont.Size() - namefont.Size() + 6
			+ ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading))
				/ 2)
			+ (finfo.ascent + finfo.descent));

	BString timeText(fTimeText);
	view->TruncateString(&timeText, B_TRUNCATE_END, rect.Width() - offset - 2);
	view->DrawString(timeText.String());

	// event name
	view->SetHighColor(textColor);

	uint16 face = 0;
	namefont.SetSize(timefont.Size() + 2);
	namefont.GetHeight(&finfo);
	if (fEvent->GetStatus() & EVENT_CANCELLED)
		face |= B_ITALIC_FACE;
	if (fEvent->GetStatus() & EVENT_DELETED)
		face |= B_STRIKEOUT_FACE;
	if (face != 0)
		namefont.SetFace(face | B_LIGHT_FACE);
	view->SetFont(&namefont);

	view->MovePenTo(offset,
		rect.top
			+ ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading))
				/ 2)
			+ (finfo.ascent + finfo.descent) - timefont.Size() + 2);

	BString name(fEvent->GetName());
	view->TruncateString(&name, B_TRUNCATE_END, rect.Width() - offset - 2);
	view->DrawString(name.String());

	// draw lines

	view->SetHighColor(
		tint_color(ui_color(B_CONTROL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
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


Event*
EventListItem::GetEvent()
{
	return fEvent;
}


void
EventListItem::_CalcTimeText(int32 mode)
{
	BString startDay, endDay, startTime, endTime, remaining, timePeriod;
	BTimeFormat timeFormat;
	BDateFormat dateFormat;
	BDateTime now = BDateTime::CurrentDateTime(B_LOCAL_TIME);

	uint16 status = fEvent->GetStatus();
	if (status & EVENT_CANCELLED)
		timePeriod << B_TRANSLATE("[Cancelled] ");
	if (status & EVENT_HIDDEN)
		timePeriod << B_TRANSLATE("[Hidden] ");
	if (status & EVENT_DELETED)
		timePeriod << B_TRANSLATE("[Deleted] ");

	if (fEvent->IsAllDay() == true)
		if ((mode & kAgendaView) || (mode & kDateView)) {
			dateFormat.Format(
				startDay, fEvent->GetStartDateTime(), B_SHORT_DATE_FORMAT);
			BString startday(B_TRANSLATE("All day, %startDay%"));
			startday.ReplaceAll("%startDay%", startDay);
			timePeriod << startday;
		} else
			timePeriod << B_TRANSLATE("All day");
	else {
		timeFormat.Format(
			startTime, fEvent->GetStartDateTime(), B_SHORT_TIME_FORMAT);
		timeFormat.Format(
			endTime, fEvent->GetEndDateTime(), B_SHORT_TIME_FORMAT);

		if (mode & kAgendaView) {
			BDurationFormat formatter(", ", B_TIME_UNIT_ABBREVIATED);
			if (now.Time_t() >= fEvent->GetStartDateTime()
				&& now.Time_t() <= fEvent->GetEndDateTime()) {
				formatter.Format(remaining, 0,
					difftime(fEvent->GetEndDateTime(), now.Time_t()) * 1000000);
				BString timeLeft(B_TRANSLATE("Now, %remaining% left"));
				timeLeft.ReplaceAll("%remaining%", remaining);
				timePeriod << timeLeft;
			} else if (now.Time_t() < fEvent->GetStartDateTime()) {
				formatter.Format(remaining, 0,
					difftime(fEvent->GetStartDateTime(), now.Time_t())
						* 1000000);
				BString timeLeft(B_TRANSLATE("Starts in %remaining%"));
				timeLeft.ReplaceAll("%remaining%", remaining);
				timePeriod << timeLeft;
			} else
				timePeriod = B_TRANSLATE("Finished!");
		} else if (mode & kDateView) {
			dateFormat.Format(
				startDay, fEvent->GetStartDateTime(), B_SHORT_DATE_FORMAT);
			dateFormat.Format(
				endDay, fEvent->GetEndDateTime(), B_SHORT_DATE_FORMAT);
			timePeriod << startTime << ", " << startDay << " - " << endTime
					   << ", " << endDay;
		} else
			timePeriod << startTime << " - " << endTime;
	}

	fTimeText = timePeriod;
}
