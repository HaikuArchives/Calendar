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
		"CREATE TABLE CATEGORIES(ID UNSIGNED INTEGER PRIMARY KEY, NAME TEXT, COLOR TEXT);"
		"CREATE TABLE EVENTS(ID TEXT PRIMARY KEY, NAME TEXT, PLACE TEXT,"
		"DESCRIPTION TEXT, ALLDAY INTEGER, START DATETIME, END DATETIME, CATEGORY UNSIGNED INTEGER,"
		"FOREIGN KEY(CATEGORY) REFERENCES CATEGORIES(ID) ON DELETE RESTRICT);"
		"INSERT INTO CATEGORIES VALUES(1, 'Default', '1e90ff');"
		"INSERT INTO CATEGORIES VALUES(2, 'Birthday', 'c25656');"
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

    sqlite3_bind_text(stmt, 1, event->GetId(), strlen(event->GetId()), 0);
    sqlite3_bind_text(stmt, 2, event->GetName(), strlen(event->GetName()), 0);
    sqlite3_bind_text(stmt, 3, event->GetPlace(), strlen(event->GetPlace()), 0);
    sqlite3_bind_text(stmt, 4, event->GetDescription(), strlen(event->GetDescription()), 0);
    sqlite3_bind_int(stmt, 5, allday);
    sqlite3_bind_int(stmt, 6, event->GetStartDateTime().Time_t());
    sqlite3_bind_int(stmt, 7, event->GetEndDateTime().Time_t());
    sqlite3_bind_int(stmt, 8, event->GetCategory()->GetId());

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

    int rc = sqlite3_prepare_v2(db, "UPDATE EVENTS SET ID=?, NAME=?, PLACE=?, DESCRIPTION=?, \
    	ALLDAY = ?, START=?, END=?, CATEGORY=?, WHERE ID=?;", -1, &stmt, NULL);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
        sqlite3_free(zErrMsg);
        return false;
    }

    int allday = (newEvent->IsAllDay())? 1 : 0;

    sqlite3_bind_text(stmt, 1, newEvent->GetId(), strlen(newEvent->GetId()), 0);
    sqlite3_bind_text(stmt, 2, newEvent->GetName(), strlen(newEvent->GetName()), 0);
    sqlite3_bind_text(stmt, 3, newEvent->GetPlace(), strlen(newEvent->GetPlace()), 0);
    sqlite3_bind_text(stmt, 4, newEvent->GetDescription(), strlen(newEvent->GetDescription()), 0);
    sqlite3_bind_int(stmt, 5, allday);
    sqlite3_bind_int(stmt, 6, newEvent->GetStartDateTime().Time_t());
    sqlite3_bind_int(stmt, 7, newEvent->GetEndDateTime().Time_t());
    sqlite3_bind_int(stmt, 8, newEvent->GetCategory()->GetId());
    sqlite3_bind_text(stmt, 9, event->GetId(), strlen(event->GetId()), 0);

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

	time_t start = BDateTime(date, BTime(0, 0, 0)).Time_t();
	time_t end = BDateTime(date, BTime(23, 59, 59)).Time_t();

	sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, "SELECT * FROM EVENTS WHERE (START >= ? AND START <= ?) \
    	OR (START < ? AND END > ?);", -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        return events;
    }

    sqlite3_bind_int(stmt, 1, start);
    sqlite3_bind_int(stmt, 2, end);
    sqlite3_bind_int(stmt, 3, start);
    sqlite3_bind_int(stmt, 4, start);

    while (rc = sqlite3_step(stmt) == SQLITE_ROW) {
        const char* id = (const char*)sqlite3_column_text(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* place = (const char*)sqlite3_column_text(stmt, 2);
        const char* description = (const char*)sqlite3_column_text(stmt, 3);

        bool allday = ((int)sqlite3_column_int(stmt, 4))? true : false;
        time_t start = (time_t)sqlite3_column_int(stmt, 5);
        time_t end = (time_t)sqlite3_column_int(stmt, 6);

        Category* category = GetCategory((uint32)sqlite3_column_int(stmt, 7));
        if (category == NULL) {
            fprintf(stderr, "Error: Received NULL category\n");
            continue;
        }

        BDateTime startDateTime, endDateTime;
        startDateTime.SetTime_t(start);
        endDateTime.SetTime_t(end);

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

    sql.SetToFormat("INSERT INTO CATEGORIES VALUES(%u, '%s', '%s');",
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

    sql.SetToFormat("UPDATE CATEGORIES SET NAME='%s', COLOR='%s' WHERE ID=%u;",
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
SQLiteManager::GetCategory(uint32 id)
{
   sqlite3_stmt* stmt;
   BString sql;

   sql.SetToFormat("SELECT * FROM CATEGORIES WHERE ID = %u;", id);

    int rc = sqlite3_prepare_v2(db, sql.String(), -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Category* category = new Category(sqlite3_column_int(stmt, 0),
        	(const char*)sqlite3_column_text(stmt, 1),
        	(const char*)sqlite3_column_text(stmt, 2));
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

        Category *category = new Category( sqlite3_column_int(stmt, 0),
             (const char*)sqlite3_column_text(stmt, 1),
             (const char*)sqlite3_column_text(stmt, 2));

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

    sql.SetToFormat("PRAGMA foreign_keys = ON; DELETE FROM CATEGORIES WHERE ID = %u;",
    	category->GetId());

    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}
