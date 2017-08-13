/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <List.h>
#include <Notification.h>
#include <String.h>
#include <TimeFormat.h>

#include "App.h"
#include "Event.h"
#include "ResourceLoader.h"
#include "SQLiteManager.h"


int32
NotificationLoop(void* data)
{
	SQLiteManager dbManager;
	BString notificationContent;
	BString startTime;
	BList* events;
	Event* event;
	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetGroup(kAppName);
	notification.SetTitle("Reminder");
	notification.SetIcon(LoadVectorIcon("BEOS:ICON", 32, 32));

	while (true)
	{
		events = dbManager.GetEventsToNotify(BDateTime::CurrentDateTime(B_LOCAL_TIME));
		for (int32 i = 0; i < events->CountItems(); i++) {
			event = ((Event*)events->ItemAt(i));
			startTime = "";
			notificationContent = "";
			if (!event->IsNotified()) {
				BTimeFormat().Format(startTime, event->GetStartDateTime(),
					B_SHORT_TIME_FORMAT);
				notificationContent.SetToFormat("%s is starting at %s.",
					event->GetName(), startTime.String());
				notification.SetContent(notificationContent);
				notification.Send();
				dbManager.UpdateNotifiedEvent(event->GetId());
				snooze(1000000);
			}
		}

		delete events;

		snooze(30000000);
	}
}
