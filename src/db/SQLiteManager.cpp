/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <stdio.h>
#include <time.h>

#include <Alert.h>
#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <List.h>
#include <Message.h>
#include <String.h>

#include "Category.h"
#include "Event.h"
#include "SQLiteManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SQLiteManager"

const char* kDirectoryName	= "Calendar";
const char* kDatabaseName	= "events.sql";


SQLiteManager::SQLiteManager()
{
	_Initialise();
}


SQLiteManager::~SQLiteManager()
{
	sqlite3_close(db);
}

void
SQLiteManager::_Initialise()
{
	BPath databasePath;
	int rc;
	char* zErrMsg = 0;
	bool exists = true;


	find_directory(B_USER_SETTINGS_DIRECTORY, &databasePath);
	databasePath.Append(kDirectoryName);
	BDirectory databaseDir(databasePath.Path());
	if (databaseDir.InitCheck() == B_ENTRY_NOT_FOUND) {
		databaseDir.CreateDirectory(databasePath.Path(), &databaseDir);
	}

	fDatabaseFile.SetTo(&databaseDir, kDatabaseName);
	if (!BEntry(fDatabaseFile.Path()).Exists())
		exists = false;

	rc = sqlite3_open(fDatabaseFile.Path(), &db);

	if (rc != SQLITE_OK) {
		sqlite3_free(zErrMsg);
		sqlite3_close(db);

		BAlert* alert = new BAlert(B_TRANSLATE("SQLITE ERROR"),
			B_TRANSLATE("Cannot open database. Your saved events will not be loaded."),
			B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
			alert->Go();
	}

	if (!exists) {
		BString sql_tmp = 
		"CREATE TABLE CATEGORIES(ID TEXT PRIMARY KEY, NAME TEXT NOT NULL UNIQUE, COLOR TEXT NOT NULL UNIQUE);"
		"CREATE TABLE EVENTS(ID TEXT PRIMARY KEY, NAME TEXT, PLACE TEXT,"
		"DESCRIPTION TEXT, ALLDAY INTEGER, START INTEGER, END INTEGER, CATEGORY TEXT, EVENT_NOTIFIED INTEGER,"
		"UPDATED INTEGER, STATUS INTEGER,"
		"FOREIGN KEY(CATEGORY) REFERENCES CATEGORIES(ID) ON DELETE RESTRICT);"
		"INSERT INTO CATEGORIES VALUES('1f1e4ffd-527d-4796-953f-df2e2c600a09', '%default%', '1E90FF');"
		"INSERT INTO CATEGORIES VALUES('47c30a47-7c79-4d45-883a-8f45b9ddcff4', '%birthday%', 'C25656');"
		"PRAGMA foreign_keys = ON;";
		sql_tmp.ReplaceAll("%default%", B_TRANSLATE("Default"));
		sql_tmp.ReplaceAll("%birthday%", B_TRANSLATE("Birthday"));

		const char *sql = sql_tmp.String();

		rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

		if (rc != SQLITE_OK ) {
			sqlite3_free(zErrMsg);
			sqlite3_close(db);

			BAlert* alert = new BAlert(B_TRANSLATE("SQLITE ERROR"),
				B_TRANSLATE("There was a SQLite error"),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
			alert->Go();
		}
	}
}


bool
SQLiteManager::AddEvent(Event* event)
{
	sqlite3_stmt* stmt;
	char* zErrMsg = 0;

	if ((BString(event->GetName()).CountChars() < 3) || (event->GetStartDateTime() >
		event->GetEndDateTime()) || (event->GetCategory() == NULL))
		return false;

	int rc = sqlite3_prepare_v2(db, "INSERT INTO EVENTS VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
		, -1, &stmt, NULL);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return false;
	}

	int allday = (event->IsAllDay())? 1 : 0;
	int notified = (event->IsNotified())? 1 : 0;
	int status = (event->GetStatus())? 1 : 0;

	sqlite3_bind_text(stmt, 1, event->GetId(), strlen(event->GetId()), 0);
	sqlite3_bind_text(stmt, 2, event->GetName(), strlen(event->GetName()), 0);
	sqlite3_bind_text(stmt, 3, event->GetPlace(), strlen(event->GetPlace()), 0);
	sqlite3_bind_text(stmt, 4, event->GetDescription(), strlen(event->GetDescription()), 0);
	sqlite3_bind_int(stmt, 5, allday);
	sqlite3_bind_int(stmt, 6, event->GetStartDateTime());
	sqlite3_bind_int(stmt, 7, event->GetEndDateTime());
	sqlite3_bind_text(stmt, 8, event->GetCategory()->GetId(),
		strlen(event->GetCategory()->GetId()), 0);
	sqlite3_bind_int(stmt, 9, notified);
	sqlite3_bind_int(stmt, 10, event->GetUpdated());
	sqlite3_bind_int(stmt, 11, status);

	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE ) {
		fprintf(stderr, "SQL error in commit: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}


bool
SQLiteManager::UpdateEvent(Event* event, Event* newEvent)
{
	sqlite3_stmt* stmt;
	char* zErrMsg = 0;

	if ((BString(newEvent->GetName()).CountChars() < 3) || (newEvent->GetStartDateTime() >
		newEvent->GetEndDateTime()) || (newEvent->GetCategory() == NULL))
		return false;

	int rc = sqlite3_prepare_v2(db, "UPDATE EVENTS SET NAME=?, PLACE=?, DESCRIPTION=?, \
		ALLDAY = ?, START=?, END=?, CATEGORY=?, EVENT_NOTIFIED=?, UPDATED=?, STATUS=? WHERE ID=?;",
		-1, &stmt, NULL);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	int allday = (newEvent->IsAllDay())? 1 : 0;
	int notified = (newEvent->IsNotified())? 1 : 0;
	int status = (newEvent->GetStatus())? 1 : 0;

	sqlite3_bind_text(stmt, 1, newEvent->GetName(), strlen(newEvent->GetName()), 0);
	sqlite3_bind_text(stmt, 2, newEvent->GetPlace(), strlen(newEvent->GetPlace()), 0);
	sqlite3_bind_text(stmt, 3, newEvent->GetDescription(), strlen(newEvent->GetDescription()), 0);
	sqlite3_bind_int(stmt, 4, allday);
	sqlite3_bind_int(stmt, 5, newEvent->GetStartDateTime());
	sqlite3_bind_int(stmt, 6, newEvent->GetEndDateTime());

	sqlite3_bind_text(stmt, 7, newEvent->GetCategory()->GetId(),
		strlen(newEvent->GetCategory()->GetId()), 0);
	sqlite3_bind_int(stmt, 8, notified);
	sqlite3_bind_int(stmt, 9, newEvent->GetUpdated());
	sqlite3_bind_int(stmt, 10, status);
	sqlite3_bind_text(stmt, 11, event->GetId(), strlen(event->GetId()), 0);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE ) {
		fprintf(stderr, "SQL error in commit: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;;
}


bool
SQLiteManager::UpdateNotifiedEvent(const char* id)
{
	char* zErrMsg = 0;
	BString sql;

	sql.SetToFormat("UPDATE EVENTS SET EVENT_NOTIFIED=1 WHERE ID='%s';", id);

	int rc = sqlite3_exec(db, sql.String(), 0, 0, &zErrMsg);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;

}


bool
SQLiteManager::RemoveEvent(Event* event)
{
	sqlite3_stmt* stmt;
	char* zErrMsg = 0;

	int rc = sqlite3_prepare_v2(db, "DELETE FROM EVENTS WHERE ID=?", -1, &stmt, NULL);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	sqlite3_bind_text(stmt, 1, event->GetId(), strlen(event->GetId()), 0);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE ) {
		fprintf(stderr, "SQL error in commit: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}


bool
SQLiteManager::RemoveCancelledEvents()
{
	sqlite3_stmt* stmt;
	char* zErrMsg = 0;

	int rc = sqlite3_prepare_v2(db, "DELETE FROM EVENTS WHERE STATUS=?",
		-1, &stmt, NULL);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	sqlite3_bind_int(stmt, 1, 0);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE ) {
		fprintf(stderr, "SQL error in commit: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}


Event*
SQLiteManager::GetEvent(const char* id)
{
	sqlite3_stmt* stmt;
	BString sql;

	sql.SetToFormat("SELECT * FROM EVENTS WHERE ID = '%s';", id);

	int rc = sqlite3_prepare_v2(db, sql.String(), -1, &stmt, NULL);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		return NULL;
	}

	if (rc = sqlite3_step(stmt) == SQLITE_ROW) {
		const char* uuid = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* place = (const char*)sqlite3_column_text(stmt, 2);
		const char* description = (const char*)sqlite3_column_text(stmt, 3);
		bool allday = ((int)sqlite3_column_int(stmt, 4))? true : false;
		time_t start = (time_t)sqlite3_column_int(stmt, 5);
		time_t end = (time_t)sqlite3_column_int(stmt, 6);

		Category* category = GetCategory((const char*)sqlite3_column_text(stmt, 7));
		if (category == NULL) {
			fprintf(stderr, "Error: Received NULL category\n");
			return NULL;
		}

		bool notified = ((int)sqlite3_column_int(stmt, 8))? true : false;
		time_t updated = (time_t)sqlite3_column_int(stmt, 9);
		bool status = ((int)sqlite3_column_int(stmt, 10))? true : false;
		Event* event = new Event(name, place, description, allday,
		start, end, category, notified, updated, status, uuid);

		sqlite3_finalize(stmt);
		return event;
	}
	else
		return NULL;
}


BList*
SQLiteManager::GetEventsOfDay(BDate& date)
{
	BList* events = new BList();

	BDateTime startOfDay(date, BTime(0, 0, 0));
	BDateTime endOfDay(date, BTime(23, 59, 59));

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, "SELECT * FROM EVENTS WHERE ((START >= ? AND START <= ?) \
		OR (START < ? AND END > ?)) AND (STATUS=?);", -1, &stmt, NULL);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		return events;
	}

	sqlite3_bind_int(stmt, 1, startOfDay.Time_t());
	sqlite3_bind_int(stmt, 2, endOfDay.Time_t());
	sqlite3_bind_int(stmt, 3, startOfDay.Time_t());
	sqlite3_bind_int(stmt, 4, startOfDay.Time_t());
	sqlite3_bind_int(stmt, 5, 1);

	while (rc = sqlite3_step(stmt) == SQLITE_ROW) {
		const char* id = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* place = (const char*)sqlite3_column_text(stmt, 2);
		const char* description = (const char*)sqlite3_column_text(stmt, 3);
		bool allday = ((int)sqlite3_column_int(stmt, 4))? true : false;
		time_t start = (time_t)sqlite3_column_int(stmt, 5);
		time_t end = (time_t)sqlite3_column_int(stmt, 6);

		Category* category = GetCategory((const char*)sqlite3_column_text(stmt, 7));
		if (category == NULL) {
			fprintf(stderr, "Error: Received NULL category\n");
			continue;
		}

		bool notified = ((int)sqlite3_column_int(stmt, 8))? true : false;
		time_t updated = (time_t)sqlite3_column_int(stmt, 9);
		bool status = ((int)sqlite3_column_int(stmt, 10))? true : false;
		Event* event = new Event(name, place, description, allday,
		start, end, category, notified, updated, status, id);

		events->AddItem(event);
	}

	sqlite3_finalize(stmt);
	return events;
}



BList*
SQLiteManager::GetEventsOfWeek(BDate& date)
{
	BList* events = new BList();
	date.AddDays(-date.DayOfWeek()+1);
	BDateTime startOfDay(date, BTime(0, 0, 0));
	date.AddDays(6);
	BDateTime endOfDay(date, BTime(23, 59, 59));
	
	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, "SELECT * FROM EVENTS WHERE ((START >= ? AND START <= ?) \
		OR (START < ? AND END > ?)) AND (STATUS=?);", -1, &stmt, NULL);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		return events;
	}

	sqlite3_bind_int(stmt, 1, startOfDay.Time_t());
	sqlite3_bind_int(stmt, 2, endOfDay.Time_t());
	sqlite3_bind_int(stmt, 3, startOfDay.Time_t());
	sqlite3_bind_int(stmt, 4, startOfDay.Time_t());
	sqlite3_bind_int(stmt, 5, 1);

	while (rc = sqlite3_step(stmt) == SQLITE_ROW) {
		const char* id = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* place = (const char*)sqlite3_column_text(stmt, 2);
		const char* description = (const char*)sqlite3_column_text(stmt, 3);
		bool allday = ((int)sqlite3_column_int(stmt, 4))? true : false;
		time_t start = (time_t)sqlite3_column_int(stmt, 5);
		time_t end = (time_t)sqlite3_column_int(stmt, 6);

		Category* category = GetCategory((const char*)sqlite3_column_text(stmt, 7));
		if (category == NULL) {
			fprintf(stderr, "Error: Received NULL category\n");
			continue;
		}

		bool notified = ((int)sqlite3_column_int(stmt, 8))? true : false;
		time_t updated = (time_t)sqlite3_column_int(stmt, 9);
		bool status = ((int)sqlite3_column_int(stmt, 10))? true : false;
		Event* event = new Event(name, place, description, allday,
		start, end, category, notified, updated, status, id);

		events->AddItem(event);
	}

	sqlite3_finalize(stmt);
	return events;
}

BList*
SQLiteManager::GetEventsToNotify(BDateTime dateTime)
{
	BList* events = new BList();
	sqlite3_stmt* stmt;

	time_t timestamp = dateTime.Time_t();

	int rc = sqlite3_prepare_v2(db, "SELECT * FROM EVENTS WHERE EVENT_NOTIFIED = ? AND START < ?\
		AND STATUS=?;",  -1, &stmt, NULL);


	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		return events;
	}

	sqlite3_bind_int(stmt, 1, 0);
	sqlite3_bind_int(stmt, 2, timestamp);
	sqlite3_bind_int(stmt, 3, 1);

	while (rc = sqlite3_step(stmt) == SQLITE_ROW) {
		const char* id = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* place = (const char*)sqlite3_column_text(stmt, 2);
		const char* description = (const char*)sqlite3_column_text(stmt, 3);
		bool allday = ((int)sqlite3_column_int(stmt, 4))? true : false;
		time_t start = (time_t)sqlite3_column_int(stmt, 5);
		time_t end = (time_t)sqlite3_column_int(stmt, 6);

		Category* category = GetCategory((const char*)sqlite3_column_text(stmt, 7));
		if (category == NULL) {
			fprintf(stderr, "Error: Received NULL category\n");
			continue;
		}

		bool notified = ((int)sqlite3_column_int(stmt, 8))? true : false;
		time_t updated = (time_t)sqlite3_column_int(stmt, 9);
		bool status = ((int)sqlite3_column_int(stmt, 10))? true : false;

		Event* event = new Event(name, place, description, allday,
		start, end, category, notified, updated, status, id);

		events->AddItem(event);
	}

	sqlite3_finalize(stmt);
	return events;

}


bool
SQLiteManager::AddCategory(Category* category)
{

	if (BString(category->GetName()).CountChars() < 3)
		return false;

	char* zErrMsg = 0;
	BString sql;

	sql.SetToFormat("INSERT INTO CATEGORIES VALUES('%s', '%s', '%s');",
		category->GetId(), category->GetName().String(),
		category->GetHexColor().String());

	int rc = sqlite3_exec(db, sql.String(), 0, 0, &zErrMsg);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;
}


bool
SQLiteManager::UpdateCategory(Category* category,
	Category* newCategory)
{
	if (BString(newCategory->GetName()).CountChars() < 3)
		return false;

	char* zErrMsg = 0;
	BString sql;

	sql.SetToFormat("UPDATE CATEGORIES SET NAME='%s', COLOR='%s' WHERE ID='%s';",
		newCategory->GetName().String(), newCategory->GetHexColor().String(),
		category->GetId());

	int rc = sqlite3_exec(db, sql.String(), 0, 0, &zErrMsg);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;
}


Category*
SQLiteManager::GetCategory(const char* id)
{
	sqlite3_stmt* stmt;
	BString sql;

	sql.SetToFormat("SELECT * FROM CATEGORIES WHERE ID = '%s';", id);

	int rc = sqlite3_prepare_v2(db, sql.String(), -1, &stmt, NULL);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		return NULL;
	}

	if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char* uuid = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* color = (const char*)sqlite3_column_text(stmt, 2);
		Category* category = new Category(name, color, uuid);

		sqlite3_finalize(stmt);
		return category;
	}

	else
		return NULL;
}


BList*
SQLiteManager::GetAllCategories()
{
	BList* categories = new BList();
	sqlite3_stmt* stmt;

	BString sql("SELECT * FROM CATEGORIES;");

	int rc = sqlite3_prepare_v2(db, sql.String(), -1, &stmt, NULL);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		return categories;
	}

	while (rc = sqlite3_step(stmt) == SQLITE_ROW) {
		const char* id = (const char*)sqlite3_column_text(stmt, 0);
		const char* name = (const char*)sqlite3_column_text(stmt, 1);
		const char* color = (const char*)sqlite3_column_text(stmt, 2);
		Category* category = new Category(name, color, id);

		categories->AddItem(category);
	}

	sqlite3_finalize(stmt);
	return categories;
}


bool
SQLiteManager::RemoveCategory(Category* category)
{
	char* zErrMsg = 0;
	BString sql;

	sql.SetToFormat("PRAGMA foreign_keys = ON; DELETE FROM CATEGORIES WHERE ID = '%s';",
		category->GetId());

	int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return false;
	}

	return true;
}
