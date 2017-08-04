/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H


#include <DateTime.h>
#include <DateFormat.h>
#include <Messenger.h>
#include <TimeFormat.h>
#include <Window.h>


class BBox;
class BButton;
class BCheckBox;
class BList;
class BMenu;
class BMenuField;
class BPopUpMenu;
class BRadioButton;
class BStringView;
class BTextControl;
class BTextView;
class BView;
class Category;
class Event;
class SQLiteManager;


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

	void			OnCheckBoxToggle();
	void			OnSaveClick();
	void			OnDeleteClick();
	void			CloseWindow();

	BString			GetDateString(time_t timeValue);
	BString			GetLocaleTimeString(time_t timeValue);

private:
	void			_InitInterface();
	void 			_DisableControls();
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

	SQLiteManager*	fDBManager;
};

#endif
