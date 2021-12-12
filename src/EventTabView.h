/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2021, Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef EVENT_TAB_VIEW_H
#define EVENT_TAB_VIEW_H

#include <DateTime.h>
#include <TabView.h>

class BList;
class Event;
class EventListView;
class QueryDBManager;


const uint32 kEditEventMessage = 'ksem';
const uint32 kDeleteEventMessage = 'kdem';
const uint32 kCancelEventMessage = 'kcem';
const uint32 kHideEventMessage = 'khev';
const uint32 kLaunchEventManagerToModify = 'klem';
const uint32 kEventSelected = 'kevs';
const uint32 kChangeListMode = 'kecm';
const uint32 kChangeListTab = 'kect';
const uint32 kListModeChanged = 'kemc';
const uint32 kListTabChanged = 'ketc';

enum {
	kAgendaView		= 1,
	kDateView		= 2,
	kHiddenView		= 4
};

enum {
	kDayTab,
	kWeekTab,
	kMonthTab
};


class EventTabView : public BTabView {
public:
					EventTabView(const BDate& date);

	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage* message);
	virtual void	Select(int32 index);

			void	SetDate(const BDate& date);
			void	ToggleMode(uint8 flag);
			uint8	Mode();

			void	SetPopUpEnabled(bool state);

			Event*	SelectedEvent();
			void	LoadEvents();

	EventListView*	ListAt(int32 index);

private:
			void	_AddEventList(const char* name, const char* label, int32 tab);
			void	_PopulateList();

	static int		_CompareFunc(const void* a, const void* b);

			BList*	fEventList;
			BDate	fDate;
			uint8	fMode;
			bool	fPopUpEnabled;
	EventTabView*	fEventTabView;
	QueryDBManager*	fDBManager;

	static const uint32	kInvokationMessage = 1000;
};

#endif // EVENT_TAB_VIEW_H
