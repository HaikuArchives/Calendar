/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <DateTime.h>
#include <View.h>


class BScrollView;
class BList;
class Event;
class EventListView;
class QueryDBManager;


const uint32 kEditEventMessage = 'ksem';
const uint32 kDeleteEventMessage = 'kdem';
const uint32 kCancelEventMessage = 'kcem';
const uint32 kLaunchEventManagerToModify = 'klem';
const uint32 kEventSelected = 'kevs';

static const int 	kDayView 		= 1006;
static const int 	kWeekView		= 1007;
static const int 	kAgendaView		= 1008;

class DayView: public BView {
public:
					DayView(const BDate& date);
		void			MessageReceived(BMessage* message);
		void			AttachedToWindow();

		void			SetDate(const BDate& date);
		void			LoadEvents();
		void			SetMode(int32 mode);
		void			SetEventListPopUpEnabled(bool state);
		static	int		CompareFunc(const void* a, const void* b);

private:
		void			_PopulateEvents();

		static const uint32 kInvokationMessage = 1000;

		BList*			fEventList;
		EventListView*	fEventListView;
		BScrollView*	fEventScroll;
		BDate			fDate;
		QueryDBManager*	fDBManager;

		int32 fMode;

};


#endif
