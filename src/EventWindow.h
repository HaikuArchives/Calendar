/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include <Button.h>
#include <Box.h>
#include <CheckBox.h>
#include <DateFormat.h>
#include <DateTime.h>
#include <List.h>
#include <Menu.h>
#include <MenuField.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <StringView.h>
#include <TextControl.h>
#include <TimeFormat.h>
#include <View.h>
#include <Window.h>

#include "CalendarMenuWindow.h"
#include "Category.h"
#include "Event.h"


const uint32 kEventWindowQuitting = 'kewq';
const uint32 kShowPopUpCalendar = 'kspc';


class EventWindow: public BWindow {
public:
			EventWindow();

	virtual void		MessageReceived(BMessage* message);
	virtual bool		QuitRequested();

	void			SetEvent(Event* event, int eventIndex,
					BList* eventList);
	void			SetEventDate(BDate& date);
	void			SetStartDate(BDate& date);
	void			SetEndDate(BDate& date);

	Event*			GetEvent();

	void 			DisableControls();
	void			OnCheckBoxToggle();
	void			OnSaveClick();
	void			OnDeleteClick();
	void			CloseWindow();

	BString			GetDateString(time_t timeValue);
	BString			GetLocaleTimeString(time_t timeValue);

private:
	void			_ShowPopUpCalendar(int8 which);

	static const uint32	kDeletePressed	= 1000;
	static const uint32	kCancelPressed	= 1001;
	static const uint32	kSavePressed	= 1002;
	static const uint32	kAllDayPressed	= 1003;
	static const uint32	kOptEveryMonth	= 1004;
	static const uint32	kOptEveryYear	= 1005;

	BTextControl*		fTextName;
	BTextControl*		fTextPlace;
	BTextControl*		fTextStartDate;
	BTextControl*		fTextEndDate;
	BTextControl*		fTextStartTime;
	BTextControl*		fTextEndTime;

	BTextView*		fTextDescription;
	BView*			fMainView;

	BMenu*			fCategoryMenu;
	BMenu*			fStartDateEdit;
	BMenu*			fEndDateEdit;

	BMenuField*		fCategoryMenuField;

	BStringView*		fNameLabel;
	BStringView*		fPlaceLabel;
	BStringView*		fDescriptionLabel;
	BStringView*		fCategoryLabel;
	BStringView*		fAllDayLabel;
	BStringView*		fRecurrenceLabel;
	BStringView*		fStartDateLabel;
	BStringView*		fStartTimeLabel;
	BStringView*		fEndDateLabel;
	BStringView*		fEndTimeLabel;

	BButton*		fDeleteButton;
	BButton*		fStartCalButton;
	BButton*		fEndCalButton;

	BRadioButton*		fEveryMonth;
	BRadioButton*		fEveryYear;

	BCheckBox*		fAllDayCheckBox;
	BCheckBox*		fStartTimeCheckBox;
	BCheckBox*		fEndTimeCheckBox;

	BBox*			fStartDateBox;
	BBox*			fEndDateBox;

	BMessenger		fCalendarWindow;

	BDateTime		fStartDateTime;
	BDateTime		fEndDateTime;

	BDateFormat		fDateFormat;
	BTimeFormat		fTimeFormat;

	Event*			fEvent;
	BList*			fEventList;
	BList*			fCategoryList;
	int			fEventIndex;

};

#endif
