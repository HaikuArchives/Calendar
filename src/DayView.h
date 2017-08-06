/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <DateTime.h>
#include <TimeFormat.h>
#include <View.h>


class BScrollView;
class BList;
class Event;
class EventListView;
class SQLiteManager;


const uint32 kModifyEventMessage = 'ksem';


class DayView: public BView {
public:
					DayView(const BDate& date);
		void			MessageReceived(BMessage* message);
		void			AttachedToWindow();

		void			SetDate(const BDate& date);
		void			LoadEvents();
		static	int		CompareFunc(const void* a, const void* b);

private:
		void			_PopulateEvents();

		static const uint32 kInvokationMessage = 1000;

		BList*			fEventList;
		EventListView*		fEventListView;
		BScrollView*		fEventScroll;
		BDate			fDate;
		BTimeFormat		fTimeFormat;
		SQLiteManager*	fDBManager;

};


#endif
