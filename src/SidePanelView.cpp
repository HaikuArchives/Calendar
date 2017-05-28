/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

 
#include "SidePanelView.h"

#include <LayoutBuilder.h>


SidePanelView::SidePanelView()
	:
	BView("SidePanelView", B_WILL_DRAW)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	
	fCalendarView = new BCalendarView("calendar");
	fCalendarView->SetWeekNumberHeaderVisible(false);
	fCalendarView->SetInvocationMessage(new BMessage(kInvokationMessage));
	fCalendarView->SetSelectionMessage(new BMessage(kInvokationMessage));
	
	fDateHeaderView = new DateHeaderView();
	
	fYearLabel = new BStringView("year", "");
	fMonthLabel = new BStringView("month", "");
	
	fMonthUpButton = new BButton("MonthuUp", ">", new BMessage(kMonthUpMessage));
	fMonthDownButton = new BButton("MonthDown", "<", new BMessage(kMonthDownMessage));
	
	fMonthUpButton->SetFlat(true);
	fMonthDownButton->SetFlat(true);
	
	float width, height;
	fMonthLabel->GetPreferredSize(&width, &height);
	fMonthLabel->SetExplicitMinSize(BSize(be_plain_font->StringWidth("September"), height));
	
	fMonthUpButton->SetExplicitMinSize(BSize(height, height));
	fMonthDownButton->SetExplicitMinSize(BSize(height, height));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.SetInsets(15)
		.Add(fDateHeaderView)
		.AddStrut(30)
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.SetInsets(5.0, 5.0, 5.0, 5.0)
			.Add(fMonthLabel)
			.AddGlue()
			.Add(fMonthDownButton)
			.Add(fMonthUpButton)
		.End()
		.AddStrut(5)
		.Add(fCalendarView)
		.AddGlue()
	.End();
	
	UpdateDate(BDate::CurrentDate(B_LOCAL_TIME));
}


void
SidePanelView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kInvokationMessage:
		{
			int32 day, month, year;
			message->FindInt32("day", &day);
			message->FindInt32("month", &month);
			message->FindInt32("year", &year);

			UpdateDate(BDate(year, month, day));
			break;
		}

		case kMonthDownMessage:
		case kMonthUpMessage:
		{
			BDate date = fCalendarView->Date();
			date.AddMonths(kMonthDownMessage == message->what ? -1 : 1);
			UpdateDate(date);
			break;
		}

		default:
			BView::MessageReceived(message);
			break;
	}
}


void
SidePanelView::UpdateDate(const BDate& date)
{
	if (!date.IsValid())
		return;

	fCalendarView->SetDate(date);

	BString text;
	text << date.Year();
	fYearLabel->SetText(text.String());

	fMonthLabel->SetText(date.LongMonthName(date.Month()).String());
}





