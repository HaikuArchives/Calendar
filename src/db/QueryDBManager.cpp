/*
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
#include <fs_index.h>
#include <fs_info.h>
#include <List.h>
#include <Message.h>
#include <MimeType.h>
#include <Query.h>
#include <String.h>
#include <StringList.h>
#include <VolumeRoster.h>

#include "App.h"
#include "Category.h"
#include "Event.h"
#include "Preferences.h"
#include "SQLiteManager.h"
#include "QueryDBManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "QueryDBManager"

const char* kEventDir		= "events";
const char* kCancelledDir	= "cancelled";
const char* kCategoryDir	= "categories";


QueryDBManager::QueryDBManager()
{
	_Initialize();
}


QueryDBManager::~QueryDBManager()
{
	delete(fEventDir, fCategoryDir);
}


void
QueryDBManager::_Initialize()
{
	BPath rootPath;
	BPath eventPath;
	BPath cancelledPath;
	BPath categoryPath;
	BPath settingsPath;
	BPath sqlPath;

	find_directory(B_USER_SETTINGS_DIRECTORY, &rootPath);
	rootPath.Append(kDirectoryName);

	// Event directory
	eventPath = BPath(rootPath);
	eventPath.Append(kEventDir);
	BDirectory* eventDir = new BDirectory(eventPath.Path());
	fEventDir = eventDir;
	if (fEventDir->InitCheck() == B_ENTRY_NOT_FOUND) {
		fEventDir->CreateDirectory(eventPath.Path(), fEventDir);
	}

	// Cancelled directory
	cancelledPath = BPath(rootPath);
	cancelledPath.Append(kCancelledDir);
	BDirectory* cancelledDir = new BDirectory(cancelledPath.Path());
	fCancelledDir = cancelledDir;
	if (fCancelledDir->InitCheck() == B_ENTRY_NOT_FOUND) {
		fCancelledDir->CreateDirectory(cancelledPath.Path(), fCancelledDir);
	}

	// Category directory
	categoryPath = BPath(rootPath);
	categoryPath.Append(kCategoryDir);
	BDirectory* categoryDir = new BDirectory(categoryPath.Path());
	fCategoryDir = categoryDir;
	if (fCategoryDir->InitCheck() == B_ENTRY_NOT_FOUND) {
		fCategoryDir->CreateDirectory(categoryPath.Path(), fCategoryDir);
	}

	BVolumeRoster volRoster;
	volRoster.GetBootVolume(&fQueryVolume);

	_CategoryMimetype();
	_EventMimetype();
	_AddIndices();

	// Create default categories if need be
	if (GetAllCategories()->CountItems() == 0) {
		Category* defaultCategory =
			new Category(B_TRANSLATE("Default"), BString("1E90FF"));
		Category* birthdayCategory =
			new Category(B_TRANSLATE("Birthday"), BString("C25656"));
		AddCategory(defaultCategory);
		AddCategory(birthdayCategory);
	}

	// Migrate from SQL, if necessary
	sqlPath = BPath(rootPath);
	sqlPath.Append(kDatabaseName);
	if (BEntry(sqlPath.Path()).Exists())
		_ImportFromSQL(sqlPath);
}


bool
QueryDBManager::AddEvent(Event* event)
{
	if (BString(event->GetName()).CountChars() < 3)
		return false;
	if (GetEvent(event->GetName(), event->GetStartDateTime()) != NULL)
		return false;

	BDirectory* parentDir = fEventDir;
	BFile evFile;
	if (!event->GetStatus())
		parentDir = fCancelledDir;

	status_t result = _CreateUniqueFile(parentDir, event->GetName(), &evFile);

	if (_EventStatusSwitch(result) != B_OK)
		return false;

	return _EventToFile(event, &evFile);
}


bool
QueryDBManager::UpdateEvent(Event* event, Event* newEvent)
{
	entry_ref ref = _GetEventRef(event->GetName(), event->GetStartDateTime());
	BFile evFile = BFile(&ref, B_READ_WRITE);
	if (_EventStatusSwitch(evFile.InitCheck()) != B_OK)
		return NULL;

	return _EventToFile(newEvent, &evFile);
}


bool
QueryDBManager::UpdateNotifiedEvent(const char* id)
{
	BFile evFile = BFile();
	_GetFileOfId(id, &evFile);
	if (_EventStatusSwitch(evFile.InitCheck()) != B_OK)
		return NULL;

	Event* event = _FileToEvent(&evFile);

	event->SetNotified(true);
	return _EventToFile(event, &evFile);
}


bool
QueryDBManager::RemoveEvent(Event* event)
{
	entry_ref ref = _GetEventRef(event->GetName(), event->GetStartDateTime());
	return RemoveEvent(ref);
}


bool
QueryDBManager::RemoveEvent(entry_ref eventRef)
{
	status_t result = BEntry(&eventRef).Remove();
	if (_EventStatusSwitch(result) == B_OK)
		return true;
	return false;
}


bool
QueryDBManager::RemoveCancelledEvents()
{
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Event:Status");
	query.PushString("Cancelled");
	query.PushOp(B_EQ);

	query.Fetch();
	entry_ref ref;

	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		RemoveEvent(ref);
	}

	return true;
}


Event*
QueryDBManager::GetEvent(const char* id)
{
	BFile evFile = BFile();
	_GetFileOfId(id, &evFile);
	if (evFile.InitCheck() != B_OK)
		return NULL;

	return _FileToEvent(&evFile);
}


Event*
QueryDBManager::GetEvent(const char* name, time_t startTime)
{
	entry_ref ref = _GetEventRef(name, startTime);

	return GetEvent(ref);
}


Event*
QueryDBManager::GetEvent(entry_ref ref)
{
	BFile evFile = BFile(&ref, B_READ_ONLY);
	if (evFile.InitCheck() != B_OK)
		return NULL;

	return _FileToEvent(&evFile);
}


BList*
QueryDBManager::GetEventsOfDay(BDate& date)
{
	time_t dayStart	= BDateTime(date, BTime(0, 0, 0)).Time_t();
	time_t dayEnd	= BDateTime(date, BTime(23, 59, 59)).Time_t();

	return _GetEventsOfInterval(dayStart, dayEnd);
}


BList*
QueryDBManager::GetEventsOfWeek(BDate date)
{
	date.AddDays(-date.DayOfWeek()+1);
	time_t weekStart = BDateTime(date, BTime(0, 0, 0)).Time_t();
	date.AddDays(6);
	time_t weekEnd = BDateTime(date, BTime(23, 59, 59)).Time_t();

	return _GetEventsOfInterval(weekStart, weekEnd);
}


BList*
QueryDBManager::GetEventsOfCategory(Category* category)
{
	BList* events = new BList();
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Event:Category");
	query.PushString(category->GetName());
	query.PushOp(B_EQ);

	query.PushAttr("Event:Status");
	query.PushString("Cancelled");
	query.PushOp(B_NE);
	query.PushOp(B_AND);

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		evFile = BFile(&ref, B_READ_WRITE);
		event = _FileToEvent(&evFile);
		events->AddItem(event);
	}

	return events;
}


BList*
QueryDBManager::GetEventsToNotify(BDateTime dateTime)
{
	BList* events = new BList();
	BQuery query;
	query.SetVolume(&fQueryVolume);

	time_t time = dateTime.Time_t();

	query.PushAttr("Event:Start");
	query.PushUInt32(time);
	query.PushOp(B_LE);

	query.PushAttr("Event:Status");
	query.PushString("Unnotified");
	query.PushOp(B_EQ);
	query.PushOp(B_AND);

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		evFile = BFile(&ref, B_READ_WRITE);
		event = _FileToEvent(&evFile);
		events->AddItem(event);
	}

	return events;
}


bool
QueryDBManager::AddCategory(Category* category)
{
	if (BString(category->GetName()).CountChars() < 3)
		return false;
	if (GetCategory(category->GetName()) != NULL)
		return false;

	BString color = category->GetHexColor();
	BList* categories = GetAllCategories();
	for (int i = 0; i < categories->CountItems(); i++)
		if (color == ((Category*)categories->ItemAt(i))->GetHexColor())
			return false;

	BFile catFile = BFile();
	status_t result =
		_CreateUniqueFile(fCategoryDir, category->GetName(), &catFile);

	if (_CategoryStatusSwitch(result) != B_OK)
		return false;

	return _CategoryToFile(category, &catFile);
}


bool
QueryDBManager::UpdateCategory(Category* category, Category* newCategory)
{
	entry_ref ref = _GetCategoryRef(category->GetName());
	BFile catFile = BFile(&ref, B_READ_WRITE);
	if (_CategoryStatusSwitch(catFile.InitCheck()) != B_OK)
		return NULL;

	if (category->GetName() != newCategory->GetName())
		_ReplaceCategory(category->GetName(), newCategory->GetName());

	return _CategoryToFile(newCategory, &catFile);
}


Category*
QueryDBManager::GetCategory(const char* name)
{
	entry_ref ref = _GetCategoryRef(name);

	return GetCategory(ref);
}


Category*
QueryDBManager::GetCategory(entry_ref ref)
{
	BFile catFile = BFile(&ref, B_READ_ONLY);

	if (catFile.InitCheck() != B_OK)
		return NULL;

	return _FileToCategory(&catFile);
}


Category*
QueryDBManager::EnsureCategory(const char* name)
{
	Category* category = GetCategory(name);

	if (category == NULL || category->GetName() == NULL) {
		category = new Category(name, BString("1E90FF"), "");
		AddCategory(category);
	}

	return category;
}


BList*
QueryDBManager::GetAllCategories()
{
	BList* categories = new BList();
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Category:Name");
	query.PushString("*");
	query.PushOp(B_EQ);

	entry_ref ref;
	query.Fetch();

	BFile catFile;
	Category* category;
	BString defaultCat = ((App*)be_app)->GetPreferences()->fDefaultCategory;

	while (query.GetNextRef(&ref) == B_OK) {
		catFile = BFile(&ref, B_READ_ONLY);
		category = _FileToCategory(&catFile);

		if (category->GetName() == defaultCat)
			categories->AddItem(category, 0);
		else if (!category->GetName().IsEmpty())
			categories->AddItem(category);
	}

	return categories;
}


bool
QueryDBManager::RemoveCategory(Category* category)
{
	entry_ref ref = _GetCategoryRef(category->GetName());
	return RemoveCategory(ref);
}


bool
QueryDBManager::RemoveCategory(entry_ref categoryRef)
{
	BEntry entry = BEntry(&categoryRef);
	BString catName = BString();
	BNode(&entry).ReadAttrString("Category:Name", &catName);

	BList* ev = GetEventsOfCategory(new Category(catName, BString("FFFFFF")));
	if (ev->CountItems() > 0)
		return false;

	if (_CategoryStatusSwitch(entry.Remove()) == B_OK)
		return true;
	return false;
}


entry_ref
QueryDBManager::_GetEventRef(const char* name, time_t startDate)
{
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Event:Name");
	query.PushString(name);
	query.PushOp(B_EQ);

	query.PushAttr("Event:Start");
	query.PushUInt32(startDate);
	query.PushOp(B_EQ);
	query.PushOp(B_AND);

	entry_ref ref;

	if (query.Fetch() == B_OK)
		query.GetNextRef(&ref);

	return ref;
}


entry_ref
QueryDBManager::_GetCategoryRef(const char* name)
{
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Category:Name");
	query.PushString(name);
	query.PushOp(B_EQ);

	entry_ref ref;

	if (query.Fetch() == B_OK)
		query.GetNextRef(&ref);

	return ref;
}


BList*
QueryDBManager::_GetEventsOfInterval(time_t start, time_t end)
{
	BList* events = new BList();
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Event:End");
	query.PushUInt32(start);
	query.PushOp(B_GE);
	query.PushAttr("Event:Start");
	query.PushUInt32(end);
	query.PushOp(B_LE);
	query.PushOp(B_AND);

	query.PushAttr("Event:Status");
	query.PushString("Cancelled");
	query.PushOp(B_NE);
	query.PushOp(B_AND);

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		evFile = BFile(&ref, B_READ_WRITE);
		event = _FileToEvent(&evFile);
		events->AddItem(event);
	}

	return events;
}


status_t
QueryDBManager::_GetFileOfId(const char* id, BFile* file)
{
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Calendar:ID");
	query.PushString(id);
	query.PushOp(B_EQ);

	entry_ref ref;
	status_t result = query.Fetch();

	if (result == B_OK) {
		entry_ref ref;
		result = query.GetNextRef(&ref);

		if (result == B_OK)
			*file = BFile(&ref, B_READ_WRITE);
	}

	return result;
}


Category*
QueryDBManager::_FileToCategory(BFile* file)
{
	BString name = BString();
	BString idStr = BString();
	file->ReadAttrString("Category:Name", &name);
	file->ReadAttrString("Calendar:ID", &idStr);

	rgb_color color = {28, 144, 255};
	file->ReadAttr("Category:Color", B_RGB_COLOR_TYPE, 0, &color,
		sizeof(rgb_color));

	return new Category(name, color, idStr.String());
}


Event*
QueryDBManager::_FileToEvent(BFile* file)
{
	BString name  = BString();
	BString catName = BString();
	BString idStr = BString();
	BString desc  = BString();
	BString place = BString();
	BString status = BString();
	file->ReadAttrString("Event:Name", &name);
	file->ReadAttrString("Event:Category", &catName);
	file->ReadAttrString("Calendar:ID", &idStr);
	file->ReadAttrString("Event:Description", &desc);
	file->ReadAttrString("Event:Place", &place);
	file->ReadAttrString("Event:Status", &status);

	time_t start	= time(NULL);
	time_t end		= time(NULL);
	time_t updated	= time(NULL);
	file->ReadAttr("Event:Start", B_TIME_TYPE, 0, &start, sizeof(time_t));
	file->ReadAttr("Event:End", B_TIME_TYPE, 0, &end, sizeof(time_t));
	file->ReadAttr("Event:Updated", B_TIME_TYPE, 0, &updated, sizeof(time_t));

	bool allDay = false;
	time_t dayStart	= BDateTime(BDate(start), BTime(0, 0, 0)).Time_t();
	time_t dayEnd	= BDateTime(BDate(end), BTime(23, 59, 0)).Time_t();
	if (dayStart <= start && start <= dayStart + 59
		&& dayEnd <= end && end <= dayEnd + 59)
		allDay = true;

	bool isNotified = false;
	if (status == BString("Notified"))
		isNotified = true;

	return new Event(name.String(), place.String(), desc.String(), allDay,
					start, end, EnsureCategory(catName.String()), isNotified,
					updated, true, idStr.String());
}


bool
QueryDBManager::_CategoryToFile(Category* category, BFile* file)
{
	if (BString(category->GetName()).CountChars() < 3)
		return false;

	if (_CategoryStatusSwitch(file->InitCheck()) != B_OK)
		return false;

	BString type = BString("application/x-calendar-category");
	file->WriteAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0, type.String(),
					type.CountChars() + 1);

	BString name = BString(category->GetName());
	file->WriteAttr("Category:Name", B_STRING_TYPE, 0, name.String(),
					name.CountChars() + 1);
	
	BString id = BString(category->GetId());
	file->WriteAttr("Calendar:ID", B_STRING_TYPE, 0, id.String(),
						id.CountChars() + 1);

	rgb_color color = category->GetColor();
	file->WriteAttr("Category:Color", B_RGB_COLOR_TYPE, 0, &color,
		sizeof(rgb_color));

	return true;
}


bool
QueryDBManager::_EventToFile(Event* event, BFile* file)
{
	if (BString(event->GetName()).CountChars() < 3)
		return false;

	if (_EventStatusSwitch(file->InitCheck()) != B_OK)
		return false;

	BString status = BString("Unnotified");
	if (!event->GetStatus()) {
		status = BString("Cancelled");
		entry_ref ref = _GetEventRef(event->GetName(), event->GetStartDateTime());
		BEntry entry = BEntry(&ref);
		entry.MoveTo(fCancelledDir);
	} else if (event->IsNotified())
		status = BString("Notified");

	file->WriteAttr("Event:Status", B_STRING_TYPE, 0, status.String(),
						status.CountChars() + 1);

	BString type = BString("application/x-calendar-event");
	file->WriteAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0, type.String(),
					type.CountChars() + 1);
	
	BString name = BString(event->GetName());
	file->WriteAttr("Event:Name", B_STRING_TYPE, 0, name.String(),
					name.CountChars() + 1);

	BString id = BString(event->GetId());
	file->WriteAttr("Calendar:ID", B_STRING_TYPE, 0, id.String(),
					id.CountChars() + 1);

	BString catName = BString(event->GetCategory()->GetName());
	file->WriteAttr("Event:Category", B_STRING_TYPE, 0, catName.String(),
					catName.CountChars() + 1);

	BString place = event->GetPlace();
	file->WriteAttr("Event:Place", B_STRING_TYPE, 0, place.String(),
					place.CountChars() + 1);

	BString desc = event->GetDescription();
	file->WriteAttr("Event:Description", B_STRING_TYPE, 0, desc.String(),
					desc.CountChars() + 1);

	time_t updated = event->GetUpdated();
	file->WriteAttr("Event:Updated", B_TIME_TYPE, 0, &updated,
					sizeof(time_t));

	time_t start = event->GetStartDateTime();
	file->WriteAttr("Event:Start", B_TIME_TYPE, 0, &start, sizeof(time_t));

	time_t end = event->GetEndDateTime();
	file->WriteAttr("Event:End", B_TIME_TYPE, 0, &end, sizeof(time_t));

	return true;
}


// Replace the category of all events of oldCategory with newCategory.
void
QueryDBManager::_ReplaceCategory(BString oldCategory, BString newCategory)
{
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Event:Category");
	query.PushString(oldCategory.String());
	query.PushOp(B_EQ);

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		evFile = BFile(&ref, B_WRITE_ONLY);
		evFile.WriteAttr("Event:Category", B_STRING_TYPE, 0,
			newCategory.String(), newCategory.CountChars() + 1);
	}
}


status_t
QueryDBManager::_CategoryStatusSwitch(status_t result)
{
	switch (result)
	{
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Category file"),
				B_TRANSLATE("Couldn't open category file because the path is not specified. "
				"It usually means that the programmer made a mistake. "
				"There is nothing you can do about it. Sorry."),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Category file"),
				B_TRANSLATE("Couldn't open category file because permission was denied. "
				"It usually means that you don't have read permissions to the event "
				"file or its parent directory, usually within the settings directory."
				"Find out the file's location and try changing its permissions."),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Category file"),
				B_TRANSLATE("There is not enough memory available on your system to save the "
				"category file. If you want to have event updates saved, try closing a few"
				"applications and try again."),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
	}

	return result;
}


status_t
QueryDBManager::_EventStatusSwitch(status_t result)
{
	switch (result)
	{
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Event file"),
				B_TRANSLATE("Couldn't open event file because the path is not specified. "
				"It usually means that the programmer made a mistake. "
				"There is nothing you can do about it. Sorry."),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Event file"),
				B_TRANSLATE("Couldn't open event file because permission was denied. "
				"It usually means that you don't have read permissions to the event "
				"file or its parent directory, usually within the settings directory."
				"Find out the file's location and try changing its permissions."),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Event file"),
				B_TRANSLATE("There is not enough memory available on your system to save the "
				"event file. If you want to have event updates saved, try closing a few"
				"applications and try again."),
				B_TRANSLATE("OK"), NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
	}

	return result;
}


bool
QueryDBManager::_CategoryMimetype()
{
	BMessage info;
	BMimeType mime("application/x-calendar-category" );
	if (mime.IsInstalled())
		return true;

	mime.GetAttrInfo(&info);

	mime.SetShortDescription("Calendar Category");
	mime.SetLongDescription("Category of calendar events");
	mime.SetPreferredApp("application/x-vnd.calendar");

	_AddAttribute(info, "Category:Name",	"Name", B_STRING_TYPE, true, 300);
	_AddAttribute(info, "Category:Color",	"Color", B_RGB_COLOR_TYPE, false, 100);
	_AddAttribute(info, "Calendar:ID",		"ID", B_STRING_TYPE, true, 100);

	return mime.SetAttrInfo( &info );
}


bool
QueryDBManager::_EventMimetype()
{
	BMessage info;
	BMimeType mime("application/x-calendar-event" );
	if (mime.IsInstalled())
		return true;

	mime.GetAttrInfo(&info);

	mime.SetShortDescription("Calendar Event");
	mime.SetLongDescription("Generic calendar event");
	mime.SetPreferredApp("application/x-vnd.calendar");

	_AddAttribute(info, "Event:Name",	"Name",		B_STRING_TYPE, true, 300);
	_AddAttribute(info, "Event:Start",	"Start",	B_TIME_TYPE, true, 150);
	_AddAttribute(info, "Event:End",	"End",		B_TIME_TYPE, true, 150);
	_AddAttribute(info, "Event:Category", "Category",
													B_STRING_TYPE, false, 200);
	_AddAttribute(info, "Event:Description", "Description",
													B_STRING_TYPE, true, 200);
	_AddAttribute(info, "Event:Place",	"Place",	B_STRING_TYPE, true, 200);
	_AddAttribute(info, "Event:Updated", "Updated",	B_TIME_TYPE, true, 150);
	_AddAttribute(info, "Event:Status",	"Status",	B_STRING_TYPE, true, 50);
	_AddAttribute(info, "Calendar:ID",	"ID",		B_STRING_TYPE, true, 100);

	return mime.SetAttrInfo( &info );
}


void
QueryDBManager::_AddIndices()
{
	int32 cookie = 0;
	dev_t device;

	while ((device = next_dev(&cookie)) >= B_OK) {
		fs_info info;
		if (fs_stat_dev(device, &info) < 0
			|| (info.flags & B_FS_HAS_QUERY) == 0)
			continue;

		fs_create_index(device, "Event:Name",		B_STRING_TYPE, 0);
		fs_create_index(device, "Event:Category",	B_STRING_TYPE, 0);
		fs_create_index(device, "Event:Place",		B_STRING_TYPE, 0);
		fs_create_index(device, "Event:Description",B_STRING_TYPE, 0);
		fs_create_index(device, "Event:Start",		B_INT32_TYPE, 0);
		fs_create_index(device, "Event:End",		B_INT32_TYPE, 0);
		fs_create_index(device, "Event:Updated",	B_INT32_TYPE, 0);
		fs_create_index(device, "Event:Status",		B_STRING_TYPE, 0);
		fs_create_index(device, "Calendar:ID",		B_STRING_TYPE, 0);
		fs_create_index(device, "Category:Name",	B_STRING_TYPE, 0);
	}
}


void
QueryDBManager::_AddAttribute(BMessage& msg, const char* name,
							const char* publicName, int32 type, bool viewable,
							int32 width)
{
	msg.AddString( "attr:name", name );
	msg.AddString( "attr:public_name", publicName );
	msg.AddInt32( "attr:type", type );
	msg.AddInt32( "attr:width", width );
	msg.AddInt32( "attr:alignment", B_ALIGN_LEFT );
	msg.AddBool( "attr:extra", false );
	msg.AddBool( "attr:viewable", viewable );
	msg.AddBool( "attr:editable", true );
}


void
QueryDBManager::_ImportFromSQL(BPath dbPath)
{
	SQLiteManager* sql = new SQLiteManager();
	BList* events = sql->GetAllEvents();
	BList* categories = sql->GetAllCategories();

	for (int i = 0; i < categories->CountItems(); i++)
		AddCategory((Category*)categories->ItemAt(i));

	for (int i = 0; i < events->CountItems(); i++)
		AddEvent((Event*)events->ItemAt(i));

	delete(sql);
	BEntry(dbPath.Path()).Rename("events.sql.bak");
}


// Create a file with name similar to the given one. If a file of the given
// name already exists, then add a unique number to the end of it.
status_t
QueryDBManager::_CreateUniqueFile(BDirectory* dir, BString name,
								BFile* newFile)
{
	if (dir->Contains(name.String())) {
		int suffix = 1;
		char suffixStr[3];
		BStringList sections = BStringList();
		bool result = name.Split(" - ", true, sections);

		if (result && sections.CountStrings() > 1) {
			sscanf(sections.Last().String(), "%u", &suffix);
			if (suffix > 0)
				suffix++;

			sections.Remove(sections.CountStrings() - 1);
			name = sections.Join(" - ");
		}

		sprintf(suffixStr, "%u", suffix);
		name += " - ";
		name += suffixStr;

		return _CreateUniqueFile(dir, name, newFile);
	}	

	dir->CreateFile(name.String(), newFile, true);
	return B_OK;
}


