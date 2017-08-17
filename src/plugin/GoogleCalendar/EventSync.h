/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _EVENT_SYNC_H
#define _EVENT_SYNC_H


#include <InterfaceKit.h>
#include <String.h>

class BHandler;
class BList;
class Event;
class SQLiteManager;


#define CLIENT_SECRET "sH095g9EzY5BxwI-DHIlqVXr"
#define CLIENT_ID "581092483253-blhpqn22i1gcpcr5o42nrja50v679g4h.apps.googleusercontent.com"
#define REDIRECT_URI "urn:ietf:wg:oauth:2.0:oob"


enum EventDateType {
	kEventUpdateDate,
	kEventStartEndDate,
};


enum EventStatus {
	kCancelledEvent = 0,
	kConfirmedEvent,
	kTentativeEvent,
};


static const uint32 kAuthorizationCode = 'katm';
static const uint32 kSyncFailedMessage = 'ksfm';
static const uint32 kSyncSuccessMessage = 'kssm';


class EventSync {
	public:
									EventSync(BHandler* handler);
									~EventSync();

		void						MessageReceived(BMessage* message);

		status_t					LoadToken();
		status_t					LoadSyncToken();
		bool						Login();
		void						NextStep(BMessage* authMessage);

		void						Sync();

		status_t					AddEvent(Event* event);
		status_t					UpdateEvent(Event* event);
		status_t					DeleteEvent(Event* event);
		status_t					GetEvents();

		status_t					ParseEvent(BMessage* eventJson);

		BString						TimeToRFC3339(time_t timeT);
		time_t						RFC3339ToTime(const char* timeString,
										EventDateType type);

	private:
		static const uint32			fStatus;

		BString 					fToken;
		BString 					fRefreshToken;
		SQLiteManager*				fDBManager;
		BString						fLastSyncToken;
		BList*						fEvents;
		BHandler*					fHandler;
};

#endif
