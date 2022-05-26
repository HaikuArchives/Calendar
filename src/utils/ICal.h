/*
 * Copyight 2020
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef ICAL_H
#define ICAL_H

#include <DataIO.h>
#include <DateTime.h>
#include <List.h>
#include <TimeZone.h>


class Event;
class QueryDBManager;
class BNotification;


int32 ImportICalEvents(void* icalFilePtr);
BList* ICalToEvents(
	BPositionIO* ical, QueryDBManager* DBManager, BNotification* progress);

Event* VEventToEvent(BStringList* vevent, QueryDBManager* DBManager);

BStringList* VCardProperties(BStringList* lines);
BString VCardPropertyName(BString property);
BString VCardPropertyValue(BString property);
status_t VCardPropertyParam(BString property, const char* param, BString* out);

status_t VCardDateToBDate(BString property, BDateTime* dateTime);

bool TimeZoneInDaylightTime(BTimeZone tz, BDate date);
void DateTimeAddSeconds(BDateTime* dt, int seconds);
void DateTimeAddMinutes(BDateTime* dt, int minutes);
void DateTimeAddHours(BDateTime* dt, int hours);

#endif
