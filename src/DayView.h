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


const uint32 kModifyEventMessage = 'ksem';


class DayView: public BView {
public:
					DayView(const BDate& date, BList* eventList);
		void			MessageReceived(BMessage* message);
		void			AttachedToWindow();

		void			AddDayEvents();
		bool			CheckForEventThisDay();
		void			SortEvents();

		void			ShowPlaceHolderText();

		void			Update(const BDate& date, BList* eventList);
		static	int		CompareFunc(const void* a, const void* b);

		BString			GetLocalisedTimeString(time_t time);

		int32			GetIndexOf(Event* event);

private:

		static const uint32 kInvokationMessage = 1000;

		BList*			fEventList;
		BList*			fDayEventList;
		BListView*		fEventListView;
		BScrollView*		fEventScroll;
		BDate			fDate;

};


#endif
