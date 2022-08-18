/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include <DateTime.h>
#include <Messenger.h>
#include <Window.h>

#include "Category.h"

class BBox;
class BButton;
class BCheckBox;
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
class Preferences;
class QueryDBManager;


static const uint32 kEventWindowQuitting = 'kewq';
static const uint32 kShowPopUpCalendar = 'kspc';
static const uint32 kStartDateChanged = 'ksdc';
static const uint32 kEndDateChanged = 'kedc';

class EventWindow : public BWindow
{
public:
					EventWindow();
					~EventWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();
	virtual void	FrameMoved(BPoint newPosition);

	void			SetEvent(Event* event);
	void			SetEvent(entry_ref ref);
	void			SetEventDate(BDate& date);
	void			SetStartDate(BDate& date);
	void			SetEndDate(BDate& date);

	void			OnCheckBoxToggle();
	void			OnReminderCheckBoxToggle();
	void			OnSaveClick();
	void			OnDeleteClick();
	void			CloseWindow();

	BString			GetDateString(BDate& date);
	BString			GetLocaleTimeString(time_t timeValue);
	void			GetDateFromMessage(BMessage* message, BDate& date);

private:
	void			_InitInterface();
	void			_PopulateWithEvent(Event* event);
	void			_DisableControls();
	void			_UpdateCategoryMenu();
	void			_ShowPopUpCalendar(int8 which);

	static const uint32 kDeletePressed = 1000;
	static const uint32 kCancelPressed = 1001;
	static const uint32 kSavePressed = 1002;
	static const uint32 kAllDayPressed = 1003;
	static const uint32 kOptEveryMonth = 1004;
	static const uint32 kOptEveryYear = 1005;
	static const uint32 kReminderPressed = 1006;

	BTextControl*	fTextName;
	BTextControl*	fTextPlace;
	BTextControl*	fTextStartDate;
	BTextControl*	fTextEndDate;
	BTextControl*	fTextStartTime;
	BTextControl*	fTextEndTime;
	BTextControl*	fTextReminderTime;

	BTextView*		fTextDescription;
	BView*			fMainView;

	BMenu*			fCategoryMenu;
	BMenu*			fStartDateEdit;
	BMenu*			fEndDateEdit;

	BMenuField*		fCategoryMenuField;

	BStringView*	fNameLabel;
	BStringView*	fPlaceLabel;
	BStringView*	fDescriptionLabel;
	BStringView*	fCategoryLabel;
	BStringView*	fAllDayLabel;
	BStringView*	fStartDateLabel;
	BStringView*	fStartTimeLabel;
	BStringView*	fEndDateLabel;
	BStringView*	fEndTimeLabel;

	BButton*		fDeleteButton;
	BButton*		fStartCalButton;
	BButton*		fEndCalButton;

	BRadioButton*	fEveryMonth;
	BRadioButton*	fEveryYear;

	BCheckBox*		fAllDayCheckBox;
	BCheckBox*		fCancelledCheckBox;
	BCheckBox*		fHiddenCheckBox;
	BCheckBox*		fReminderCheckBox;

	BBox*			fStartDateBox;
	BBox*			fEndDateBox;
	BBox*			fStatusBox;

	BMessenger		fCalendarWindow;

	BDate			fStartDate;
	BDate			fEndDate;

	bool			fNew;

	Event*			fEvent;
	entry_ref		fEventRef;
	CategoryList*	fCategoryList;

	QueryDBManager*	fDBManager;
};

#endif
