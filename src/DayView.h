/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <DateTime.h>
#include <List.h>
#include <ListView.h>
#include <ScrollView.h>
#include <View.h>

#include "Event.h"


const uint32 kSelectionMessage = 'ksem';


class DayView: public BView {
public:
						DayView(const BDate& date, BList* eventList);
		void			MessageReceived(BMessage* message);

		void			AddDayEvents();
		void			CheckForEventThisDay();
		int32			CompareFunc(const void*, const void*);

		BString			GetLocalisedTimeString(time_t time);

		int32			GetIndexOf(Event* event);

private:
		BList*			fEventList;
		BList*			fDayEventList;
		BListView*		fEventListView;
		BScrollView*	fEventScroll;
		BDate			fDate;

};


#endif
