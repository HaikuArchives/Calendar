/*
 * Copyright 2020-2021 Jaidyn Levesque, <jadedctrl@tekik.io>
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _QDB_MANAGER_H_
#define _QDB_MANAGER_H_


#include <DateTime.h>
#include <Directory.h>
#include <Path.h>
#include <Volume.h>

#include "Category.h"
#include "Event.h"

class Category;
class Event;


extern const char* kDirectoryName;
extern const char* kEventDir;
extern const char* kCategoryDir;
extern const char* kDatabaseName;


class QueryDBManager
{
public:
				QueryDBManager();
				~QueryDBManager();

	bool		AddEvent(Event* event);
	bool		UpdateEvent(Event* event, Event* newEvent);
	bool		UpdateEvent(Event* event, entry_ref ref);
	bool		UpdateNotifiedEvent(const char* id);

	Event*		GetEvent(const char* id);
	Event*		GetEvent(const char* name, time_t startTime);
	Event*		GetEvent(entry_ref ref);

	EventList*	GetEventsOfDay(BDate& date, bool ignoreHidden = true);
	EventList*	GetEventsOfWeek(BDate date, bool ignoreHidden = true);
	EventList*	GetEventsOfMonth(BDate date, bool ignoreHidden = true);
	EventList*	GetEventsInInterval(
					time_t start, time_t end, bool ignoreHidden = true);
	EventList*	GetEventsOfCategory(Category* category);
	EventList*	GetEventsToNotify(BDateTime dateTime);

	bool		RemoveEvent(Event* event);
	bool		RemoveEvent(entry_ref eventRef, const char* restorePath = NULL);
	bool		RestoreEvent(entry_ref ref);

	bool		AddCategory(Category* category);
	bool		UpdateCategory(Category* category, Category* newCategory);
	Category*	GetCategory(const char* name);
	Category*	GetCategory(entry_ref ref);
	Category*	EnsureCategory(const char* name);
	CategoryList* GetAllCategories();
	bool		RemoveCategory(Category* category);
	bool		RemoveCategory(entry_ref categoryRef);

	void		SetDefaultCategory(BString cat);

private:
	void		_Initialize();

	entry_ref	_GetEventRef(const char* name, time_t startDate);
	entry_ref	_GetCategoryRef(const char* name);

	status_t	_GetFileOfId(const char* id, BFile* file, entry_ref* ref = NULL);

	Category*	_FileToCategory(BFile* file);
	Event*		_FileToEvent(entry_ref* ref);
	bool		_CategoryToFile(Category* category, BFile* file);
	bool		_EventToFile(Event* event, BFile* file);

	void		_ReplaceCategory(BString oldCategory, BString newCategory);

	status_t	_CategoryStatusSwitch(status_t);
	status_t	_EventStatusSwitch(status_t);
	status_t	_TrashStatusSwitch(status_t);
	status_t	_RestoreStatusSwitch(status_t);

	void		_ImportFromSQL(BPath dbPath);
	void		_MigrateCancellations(BPath cancelPath);

	bool		_CategoryMimetype();
	bool		_EventMimetype();
	void		_AddIndices();

	void		_AddAttribute(BMessage& msg, const char* name, const char* publicName,
					int32 type, bool viewable, int32 width);
	status_t	_CreateUniqueFile(BDirectory* dir, BString leaf, BFile* newFile);
	BString		_UniqueFilename(BDirectory* dir, BString leaf);
	BString		_UniqueEventName(
					BString name, time_t startTime, const char* id = NULL);
	BString		_IncrementSuffix(BString string);

	BPath		_SettingsPath(const char* leaf);
	BDirectory*	_EnsureDirectory(BPath path);


	BDirectory*	fEventDir;
	BDirectory*	fCategoryDir;
	BDirectory*	fTrashDir;
	BVolume		fQueryVolume;
	BString		fDefaultCat;
};

#endif // _QDB_MANAGER_H_
