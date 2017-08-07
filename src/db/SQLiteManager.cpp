/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <stdio.h>
#include <time.h>

#include <Alert.h>
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

		BAlert* alert = new BAlert("SQLITE ERROR",
			"Cannot open database. Your saved events will not be loaded.",
			"OK", NULL, NULL,
			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
			alert->Go();
	}

	if (!exists) {
		const char *sql =
		"CREATE TABLE CATEGORIES(ID TEXT PRIMARY KEY, NAME TEXT NOT NULL UNIQUE, COLOR TEXT);"
		"CREATE TABLE EVENTS(ID TEXT PRIMARY KEY, NAME TEXT, PLACE TEXT,"
		"DESCRIPTION TEXT, ALLDAY INTEGER, START DATETIME, END DATETIME, CATEGORY TEXT,"
		"FOREIGN KEY(CATEGORY) REFERENCES CATEGORIES(ID) ON DELETE RESTRICT);"
		"INSERT INTO CATEGORIES VALUES('1f1e4ffd-527d-4796-953f-df2e2c600a09', 'Default', '1e90ff');"
		"INSERT INTO CATEGORIES VALUES('47c30a47-7c79-4d45-883a-8f45b9ddcff4', 'Birthday', 'c25656');"
		"PRAGMA foreign_keys = ON;";

		rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

		if (rc != SQLITE_OK ) {
			sqlite3_free(zErrMsg);
			sqlite3_close(db);

			BAlert* alert = new BAlert("SQLITE ERROR",
				"There was a SQLite error",
				"OK", NULL, NULL,
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

    int rc = sqlite3_prepare_v2(db, "INSERT INTO EVENTS VALUES(?, ?, ?, ?, ?, ?, ?, ?);", -1, &stmt, NULL);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return false;
    }

    int allday = (event->IsAllDay())? 1 : 0;

    BDate startDate = event->GetStartDateTime().Date();
    BDate endDate = event->GetEndDateTime().Date();
    BTime startTime = event->GetStartDateTime().Time();
    BTime endTime = event->GetEndDateTime().Time();

    BString start, end;

    // TODO: Use BDateTimeFormat/BTimeFormat/BDateFormat to perform date/time parsing
    // and formatting. Also consider timezone while storing timestamp.
    start.SetToFormat("%04d-%02d-%02d %02d:%02d:%02d", startDate.Year(), startDate.Month(),
		startDate.Day(), startTime.Hour(), startTime.Minute(), 0);
    end.SetToFormat("%04d-%02d-%02d %02d:%02d:%02d", endDate.Year(), endDate.Month(),
		endDate.Day(), endTime.Hour(), endTime.Minute(), 0);

    sqlite3_bind_text(stmt, 1, event->GetId(), strlen(event->GetId()), 0);
    sqlite3_bind_text(stmt, 2, event->GetName(), strlen(event->GetName()), 0);
    sqlite3_bind_text(stmt, 3, event->GetPlace(), strlen(event->GetPlace()), 0);
    sqlite3_bind_text(stmt, 4, event->GetDescription(), strlen(event->GetDescription()), 0);
    sqlite3_bind_int(stmt, 5, allday);
    sqlite3_bind_text(stmt, 6, start, strlen(start), 0);
    sqlite3_bind_text(stmt, 7, end, strlen(end), 0);
    sqlite3_bind_text(stmt, 8, event->GetCategory()->GetId(),
		strlen(event->GetCategory()->GetId()), 0);

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
		ALLDAY = ?, START=?, END=?, CATEGORY=? WHERE ID=?;", -1, &stmt, NULL);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
        sqlite3_free(zErrMsg);
        return false;
    }

    int allday = (newEvent->IsAllDay())? 1 : 0;

    BDate startDate = newEvent->GetStartDateTime().Date();
    BDate endDate = newEvent->GetEndDateTime().Date();
    BTime startTime = newEvent->GetStartDateTime().Time();
    BTime endTime = newEvent->GetEndDateTime().Time();

    BString start, end;

    start.SetToFormat("%04d-%02d-%02d %02d:%02d:%02d", startDate.Year(), startDate.Month(),
		startDate.Day(), startTime.Hour(), startTime.Minute(), 0);
    end.SetToFormat("%04d-%02d-%02d %02d:%02d:%02d", endDate.Year(), endDate.Month(),
		endDate.Day(), endTime.Hour(), endTime.Minute(), 0);


    sqlite3_bind_text(stmt, 1, newEvent->GetName(), strlen(newEvent->GetName()), 0);
    sqlite3_bind_text(stmt, 2, newEvent->GetPlace(), strlen(newEvent->GetPlace()), 0);
    sqlite3_bind_text(stmt, 3, newEvent->GetDescription(), strlen(newEvent->GetDescription()), 0);
    sqlite3_bind_int(stmt, 4, allday);
    sqlite3_bind_text(stmt, 5, start, strlen(start), 0);
    sqlite3_bind_text(stmt, 6, end, strlen(end), 0);

    sqlite3_bind_text(stmt, 7, newEvent->GetCategory()->GetId(),
		strlen(newEvent->GetCategory()->GetId()), 0);
    sqlite3_bind_text(stmt, 8, event->GetId(), strlen(event->GetId()), 0);

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


BList*
SQLiteManager::GetEventsOfDay(BDate& date)
{
	BList* events = new BList();

	BString start, end;

    start.SetToFormat("%04d-%02d-%02d 00:00:00", date.Year(), date.Month(),
		date.Day());
    end.SetToFormat("%04d-%02d-%02d 23:59:59", date.Year(), date.Month(),
		date.Day());

	sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT * FROM EVENTS WHERE (START >= ? AND START <= ?) \
    	OR (START < ? AND END > ?);", -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        return events;
    }

    sqlite3_bind_text(stmt, 1, start, strlen(start), 0);
    sqlite3_bind_text(stmt, 2, end, strlen(end), 0);
    sqlite3_bind_text(stmt, 3, start, strlen(start), 0);
    sqlite3_bind_text(stmt, 4, start, strlen(start), 0);

    while (rc = sqlite3_step(stmt) == SQLITE_ROW) {
        const char* id = (const char*)sqlite3_column_text(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* place = (const char*)sqlite3_column_text(stmt, 2);
        const char* description = (const char*)sqlite3_column_text(stmt, 3);
        bool allday = ((int)sqlite3_column_int(stmt, 4))? true : false;
        start = (const char*)sqlite3_column_text(stmt, 5);
		end = (const char*)sqlite3_column_text(stmt, 6);

        Category* category = GetCategory((const char*)sqlite3_column_text(stmt, 7));
        if (category == NULL) {
            fprintf(stderr, "Error: Received NULL category\n");
            continue;
        }

		int year, month, day, hour, min, sec;
		sscanf(start, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        BDateTime startDateTime(BDate(year, month, day), BTime(hour, min, sec));
        sscanf(end, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
        BDateTime endDateTime(BDate(year, month, day), BTime(hour, min, sec));

        Event* event = new Event(name, place, description, allday,
        	startDateTime, endDateTime, category, id);

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
		const char* id = (const char*)sqlite3_column_text(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* color = (const char*)sqlite3_column_text(stmt, 2);
        Category* category = new Category(name, color, id);

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
