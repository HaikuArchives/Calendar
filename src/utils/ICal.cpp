/*
 * Copyight 2020
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ICal.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <Catalog.h>
#include <DataIO.h>
#include <DateTime.h>
#include <List.h>
#include <Notification.h>
#include <StringList.h>
#include <StringFormat.h>
#include <TimeZone.h>

#include "App.h"
#include "Event.h"
#include "EventTabView.h"
#include "MainWindow.h"
#include "Preferences.h"
#include "QueryDBManager.h"
#include "ResourceLoader.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ICalendar"


int32
ImportICalEvents(void* icalFilePtr)
{
	QueryDBManager* DBManager = new QueryDBManager;
	BFile* icalFile = (BFile*)icalFilePtr;
	if (icalFile->InitCheck() != B_OK) {
		return 0;
	}

	BNotification progress(B_PROGRESS_NOTIFICATION);
	progress.SetGroup(kAppName);
	progress.SetTitle(B_TRANSLATE("Importing events…"));
	progress.SetIcon(LoadVectorIcon("BEOS:ICON", 32, 32));
	progress.SetMessageID("ical_progress");
	progress.SetProgress(0.0);

	BList* iEvents = ICalToEvents(icalFile, DBManager, &progress);

	if (iEvents->CountItems() == 1) {
		BMessage* evmsg = new BMessage(kLaunchEventManagerToModify);
		evmsg->AddPointer("event", (Event*)iEvents->FirstItem());
		((App*)be_app)->mainWindow()->MessageReceived(evmsg);

	} else if (iEvents->CountItems() > 1) {
		int success = 0;
		for (int i = 0; i < iEvents->CountItems(); i++) {
			if (DBManager->AddEvent((Event*)iEvents->ItemAt(i)))
				success++;
		}

		BString progressTitle;
		static BStringFormat titleFormat(B_TRANSLATE("{0, plural,"
			"=0{No events imported.}"
			"=1{Imported one event.}"
			"other{Imported # events.}}"));
		titleFormat.Format(progressTitle, success);

		progress.SetTitle(progressTitle);
		progress.SetProgress(1.0);
		progress.Send();
	}
	delete icalFile, DBManager, iEvents;
	return 0;
}


BList*
ICalToEvents(BPositionIO* ical, QueryDBManager* DBManager,
	BNotification* progress)
{
	BList* events = new BList();
	off_t size = 0;
	char buf[1];
	bool inEvent = false;
	BStringList eventLines = BStringList();
	BString line("");

	if (ical->GetSize(&size) != B_OK)
		return events;

	float count = 0;
	while (ical->Read(&buf, 1)) {
		if (buf[0] != '\n' && buf[0] != '\r')
			line << buf[0];

		if (buf[0] == '\n') {
			if (BString(line).ToLower() == "begin:vevent")
				inEvent = true;
			if (inEvent)
				eventLines.Add(line);
			if (BString(line).ToLower() == "end:vevent") {
				inEvent = false;
				Event* event = VEventToEvent(&eventLines, DBManager);
				if (event != NULL)
					events->AddItem(event);

				// Don't bother notifying for very small imports
				if (size > 500 || progress != NULL) {
					progress->SetProgress(count / size);
					progress->Send();
				}
			}
			line.SetTo("");
		}
		count++;
	}
	return events;
}


Event*
VEventToEvent(BStringList* vevent, QueryDBManager* DBManager)
{
	BString name("");
	BString desc("");
	BString place("");
	BString uid("");
	uint16 status = 0;
	bool allDay = false;
	BDateTime start = BDateTime();
	BDateTime end = BDateTime();
	BDateTime updated = BDateTime();

	// Ignore valarm blocks
	int32 alarmStart = vevent->IndexOf(BString("begin:valarm"), true);
	int32 alarmEnd = vevent->IndexOf(BString("end:valarm"), true);
	vevent->Remove(alarmStart, alarmEnd - alarmStart);

	BStringList* properties = VCardProperties(vevent);

	BString defaultCatName = ((App*)be_app)->GetPreferences()->fDefaultCategory;
	Category* category = DBManager->GetCategory(defaultCatName.String());

	for (int i = 0; i < properties->CountStrings(); i++) {
		BString property = properties->StringAt(i);
		BString propName = VCardPropertyName(property).ToLower();
		BString propValue = VCardPropertyValue(property);

		if (propName == "summary")
			name = propValue;
		else if (propName == "description")
			desc = propValue;
		else if (propName == "location")
			place = propValue;
		else if (propName == "status" && propValue.ToLower() == "cancelled")
			status |= EVENT_CANCELLED;
		else if (propName == "uid")
			uid = propValue;

		else if (propName == "comment"
			&& !properties->HasString("description", true))
			desc = propValue;
		else if (propName == "geo"
			&& !properties->HasString("location", true))
			desc = propValue;

		else if (propName == "dtstart")
			VCardDateToBDate(property, &start);
		else if (propName == "dtend")
			VCardDateToBDate(property, &end);
		else if (propName == "last-modified")
			VCardDateToBDate(property, &updated);

		else if (propName == "dtstamp"
			&& properties->HasString("dtstart", true))
			VCardDateToBDate(property, &start);

		else if (propName == "categories") {
			BStringList catNames = BStringList();
			propValue.Split(",", false, catNames);
			for (int j = 0; j < catNames.CountStrings(); j++) {
				BString catName = catNames.StringAt(j);
				Category* catTest = DBManager->GetCategory(catName.String());
				if (catTest != NULL) {
					category = catTest;
					break;
				}
			}
		}
	}

	if (name.IsEmpty() || start.Date().Year() == 0 && end.Date().Year() == 0)
		return NULL;
	if (end.Date().Year() == 0) {
		end = start;
		if (start.Time().Hour() == 0 && start.Time().Minute() == 0)
			end.SetTime(BTime(23, 59, 0));
	}

	if (start.Time().Hour() == 0 && start.Time().Minute() == 0
		&& end.Time().Hour() == 23 && end.Time().Minute() == 59)
		allDay = true;

	delete properties;
	return new Event(name.String(), place.String(), desc.String(), allDay,
		start.Time_t(), end.Time_t(), category, updated.Time_t(), status,
		uid.String());
}


// Put all properties of vcard input into seperate strings, accounting for
// multi-lined property values.
BStringList*
VCardProperties(BStringList* lines)
{
	BStringList* props = new BStringList();

	for (int i = 0; i < lines->CountStrings(); i++) {
		BString line = lines->StringAt(i);

		if (line.StartsWith(" ")) {
			BString replacement = props->Last().Append(line);
			props->Replace(props->CountStrings() - 1, replacement);
		} else
			props->Add(line);
	}
	return props;
}


// Get the name of a property
// NAME;PARAM=PVALUE:VALUE
BString
VCardPropertyName(BString property)
{
	BStringList split = BStringList();
	property.Split(":", true, split);
	property = split.First();

	split.MakeEmpty();
	property.Split(";", true, split);
	return split.First();
}


// Get the value of a property
BString
VCardPropertyValue(BString property)
{
	BStringList split = BStringList();
	property.Split(":", true, split);
	split.Remove(0);

	BString value = split.Join(":");
	value.IReplaceAll("\\n", "\n");
	return value.CharacterDeescape('\\');
}


// Get the value of a parameter
status_t
VCardPropertyParam(BString property, const char* param, BString* out)
{
	BStringList split = BStringList();
	property.Split(":", true, split);
	property = split.First();

	BStringList params = BStringList();
	property.Split(";", true, params);
	params.Remove(0);

	BString paramStr;
	BStringList paramValue;

	for (int i = 0; i <= params.CountStrings(); i++) {
		paramStr = params.StringAt(i);
		paramStr.Split("=", false, paramValue);

		if (paramValue.First().ToLower() == BString(param).ToLower()) {
			*out = BString(paramValue.StringAt(1));
			return B_OK;
		}
		paramValue.MakeEmpty();
	}

	return B_ERROR;
}


status_t
VCardDateToBDate(BString property, BDateTime* dateTime)
{
	BString value = VCardPropertyValue(property);
	std::istringstream dateStream(value.String());
	std::tm tm = {};

	// If date-only
	BString valueType;
	status_t valueStatus = VCardPropertyParam(property, "value", &valueType);
	if (valueStatus == B_OK && valueType.ToLower() == "date") {
		if (!(dateStream >> std::get_time(&tm, "%Y%m%d")))
			return B_ERROR;
		dateTime->SetDate(BDate(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday));
		return B_OK;
	}

	if (!(dateStream >> std::get_time(&tm, "%Y%m%dT%H%M%S")))
		return B_ERROR;

	BDate date = BDate(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	BTime time = BTime(tm.tm_hour, tm.tm_min, tm.tm_sec, 0);
	dateTime->SetDateTime(date, time);

	// If a time-zone was specified, convert to UTC 
	BString tzid;
	status_t tzStatus = VCardPropertyParam(property, "tzid", &tzid);
	if (tzStatus == B_OK) {
		BTimeZone tz = BTimeZone(tzid.String());
		DateTimeAddSeconds(dateTime, -tz.OffsetFromGMT());

		if (TimeZoneInDaylightTime(tz, date))
			DateTimeAddHours(dateTime, -1);
	}

	// If time was originally UTC (or converted), convert to local time
	if (value.EndsWith("Z") || tzStatus == B_OK) {
		DateTimeAddSeconds(dateTime, BTimeZone().OffsetFromGMT());
	}
	return B_OK;
}


bool
TimeZoneInDaylightTime(BTimeZone tz, BDate date)
{
	return false;
}


// I can't find a built-in way to add seconds/etc to a BDateTime yet (to a BTime
// while also rolling over the date of a corresponding BDate), so here's this,
// for now.
void
DateTimeAddSeconds(BDateTime* dt, int seconds)
{
	int secondsInDay = 60 * 60 * 24;
	int oneDay = 1;

	if (seconds < 0) {
		secondsInDay = -secondsInDay;
		oneDay = -1;
	}

	while (abs(seconds) > abs(secondsInDay)) {
		dt->Time().AddSeconds(secondsInDay);
		seconds -= secondsInDay;
		dt->Date().AddDays(oneDay);
	}
	int oldHour = dt->Time().Hour();
	dt->Time().AddSeconds(seconds);
	int newHour = dt->Time().Hour();

	if (newHour < oldHour && secondsInDay > 0)
		dt->Date().AddDays(1);
	else if (newHour > oldHour && secondsInDay < 0)
		dt->Date().AddDays(-1);
}


void
DateTimeAddMinutes(BDateTime* dt, int minutes)
{
	DateTimeAddSeconds(dt, minutes * 60);
}


void
DateTimeAddHours(BDateTime* dt, int hours)
{
	DateTimeAddMinutes(dt, hours * 60);
}


