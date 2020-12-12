/*
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _QDB_MANAGER_H_
#define _QDB_MANAGER_H_


#include <DateTime.h>
#include <Directory.h>
#include <Path.h>
#include <Volume.h>

#include "Preferences.h"

class Category;
class Event;


extern const char* kDirectoryName;
extern const char* kEventDir;
extern const char* kCancelledDir;
extern const char* kCategoryDir;
extern const char* kDatabaseName;


class QueryDBManager {
public:
					QueryDBManager();
					~QueryDBManager();

		bool		AddEvent(Event* event);
		bool		UpdateEvent(Event* event, Event* newEvent);
		bool		UpdateNotifiedEvent(const char* id);

		Event*		GetEvent(const char* id);
		Event*		GetEvent(const char* name, time_t startTime);
		BList*		GetEventsOfDay(BDate& date);
		BList*		GetEventsOfWeek(BDate& date);
		BList*		GetEventsOfCategory(Category* category);
		BList*		GetEventsToNotify(BDateTime dateTime);
		bool		RemoveEvent(Event* event);
		bool		RemoveEvent(entry_ref eventRef);
		bool		RemoveCancelledEvents();

		bool		AddCategory(Category* category);
		bool		UpdateCategory(Category* category,
						Category* newCategory);
		Category*	GetCategory(const char* name);
		Category*	EnsureCategory(const char* name);
		BList*		GetAllCategories();
		bool		RemoveCategory(Category* category);
		bool		RemoveCategory(entry_ref categoryRef);

private:

	void			_Initialise();

	entry_ref		_GetEventRef(const char* name, time_t startDate);
	entry_ref		_GetCategoryRef(const char* name);

	BList*			_GetEventsOfInterval(time_t start, time_t end);
	status_t		_GetFileOfId(const char* id, BFile* file);

	Category*		_FileToCategory(BFile* file);
	Event*			_FileToEvent(BFile* file);
	bool			_CategoryToFile(Category* category, BFile* file);
	bool			_EventToFile(Event* event, BFile* file);

	void			_ReplaceCategory(BString oldCategory, BString newCategory);
	
	status_t		_CategoryStatusSwitch(status_t);
	status_t		_EventStatusSwitch(status_t);

	void			_ImportFromSQL(BPath dbPath);

	bool			_CategoryMimetype();
	bool			_EventMimetype();
	void			_AddIndices();

	void			_AddAttribute(BMessage& msg, const char* name,
								const char* publicName, int32 type,
								bool viewable, int32 width);
	status_t		_CreateUniqueFile(BDirectory*, BString, BFile*);


	BDirectory*		fEventDir;
	BDirectory*		fCancelledDir;
	BDirectory*		fCategoryDir;
	BVolume			fQueryVolume;

	Preferences*	fPreferences;
};
#endif // _QDB_MANAGER_H_
