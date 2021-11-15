/*
 * Copyright 2020-2021 Jaidyn Levesque, <jadedctrl@tekik.io>
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
#include "ResourceLoader.h"
#include "SQLiteManager.h"
#include "QueryDBManager.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "QueryDBManager"

const char* kEventDir		= "events";
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
	fEventDir = _EnsureDirectory(_SettingsPath(kEventDir));
	fCategoryDir = _EnsureDirectory(_SettingsPath(kCategoryDir));

	// Trash directory
	BPath trashPath;
	find_directory(B_TRASH_DIRECTORY, &trashPath);
	BDirectory* trashDir = new BDirectory(trashPath.Path());
	fTrashDir = trashDir;

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
	BPath sqlPath = _SettingsPath(kDatabaseName);
	if (BEntry(sqlPath.Path()).Exists())
		_ImportFromSQL(sqlPath);

	// Migrate pre-"Cancel/Delete"-dichtonomy events
	BPath cancelPath = _SettingsPath("cancelled");
	if (BEntry(cancelPath.Path()).Exists())
		_MigrateCancellations(cancelPath);
}


bool
QueryDBManager::AddEvent(Event* event)
{
	Event* oldEvent = GetEvent(event->GetName(), event->GetStartDateTime());
	if (oldEvent != NULL && !(oldEvent->GetStatus() & EVENT_DELETED))
		return false;

	BDirectory* parentDir = fEventDir;
	BFile evFile;
	status_t result = _CreateUniqueFile(parentDir, event->GetName(), &evFile);

	if (_EventStatusSwitch(result) != B_OK)
		return false;

	return _EventToFile(event, &evFile);
}


bool
QueryDBManager::UpdateEvent(Event* event, Event* newEvent)
{
	entry_ref ref = _GetEventRef(event->GetName(), event->GetStartDateTime());
	return UpdateEvent(newEvent, ref);
}


bool
QueryDBManager::UpdateEvent(Event* event, entry_ref ref)
{
	BFile evFile = BFile(&ref, B_READ_WRITE);
	BEntry evEntry(&ref);
	if (_EventStatusSwitch(evFile.InitCheck()) != B_OK)
		return false;

	bool ret = _EventToFile(event, &evFile);

	// If event is newly deleted, toss in bin. If undeleted, move back!
	bool inTrash = fTrashDir->Contains(&evEntry);
	if ((event->GetStatus() & EVENT_DELETED) && inTrash == false)
		return RemoveEvent(ref);
	else if (!(event->GetStatus() & EVENT_DELETED) && inTrash == true)
		return RestoreEvent(ref);

	return ret;
}


bool
QueryDBManager::UpdateNotifiedEvent(const char* id)
{
	 BFile evFile = BFile();
	 entry_ref ref;
	_GetFileOfId(id, &evFile, &ref);
	if (_EventStatusSwitch(evFile.InitCheck()) != B_OK)
		return NULL;

	Event* event = _FileToEvent(&ref);
	event->SetStatus(event->GetStatus() | EVENT_NOTIFIED);
	return _EventToFile(event, &evFile);
}


bool
QueryDBManager::RemoveEvent(Event* event)
{
	entry_ref ref = _GetEventRef(event->GetName(), event->GetStartDateTime());
	return RemoveEvent(ref);
}


bool
QueryDBManager::RemoveEvent(entry_ref eventRef, const char* restorePath)
{
	BNode node(&eventRef);
	BString path = restorePath;
	if (restorePath == NULL)
		path = BPath(&eventRef).Path();
	node.WriteAttrString("_trk/original_path", &path);

	char leafName[B_FILE_NAME_LENGTH] = {'\0'};
	BEntry entry(&eventRef);
	entry.GetName(leafName);
	BString leaf = _UniqueFilename(fTrashDir, leafName);

	BEntry trashEnt;
	fTrashDir->GetEntry(&trashEnt);
	printf("Trashing event %s to %s/%s…\n",
			path.String(), BPath(&trashEnt).Path(), leaf.String());

	return (_TrashStatusSwitch(entry.MoveTo(fTrashDir, leaf)) == B_OK);
}


bool
QueryDBManager::RestoreEvent(entry_ref ref)
{
	BEntry entry(&ref);
	BFile file(&ref, B_READ_ONLY);
	if (_RestoreStatusSwitch(entry.InitCheck()) != B_OK
		|| _RestoreStatusSwitch(file.InitCheck()) != B_OK)
		return false;

	BDirectory* parentDir = fEventDir;

	BString restorePath;
	if (file.ReadAttrString("_trk/original_path", &restorePath) == B_OK) {
		BEntry restoreEntry(restorePath.String());
		restoreEntry.GetParent(parentDir);
	}

	char oldLeaf[B_FILE_NAME_LENGTH] = {'\0'};
	entry.GetName(oldLeaf);
	BString leaf = _UniqueFilename(fEventDir, BString(oldLeaf));

	Event* event = _FileToEvent(&ref);
	if (event != NULL) {
		BString eventName = _UniqueEventName(event->GetName(),
			event->GetStartDateTime());
		event->SetName(eventName);
		_EventToFile(event, &file);
	}
	delete event;

	BEntry parentEnt;
	parentDir->GetEntry(&parentEnt);
	printf("Restoring event %s to %s…\n", BPath(&entry).Path(),
		BPath(&parentEnt).Path());

	return (_RestoreStatusSwitch(entry.MoveTo(parentDir, leaf)) == B_OK);
}


Event*
QueryDBManager::GetEvent(const char* id)
{
	entry_ref ref;
	_GetFileOfId(id, NULL, &ref);
	return _FileToEvent(&ref);
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
	return _FileToEvent(&ref);
}


BList*
QueryDBManager::GetEventsOfDay(BDate& date, bool ignoreHidden)
{
	time_t dayStart	= BDateTime(date, BTime(0, 0, 0)).Time_t();
	time_t dayEnd	= BDateTime(date, BTime(23, 59, 59)).Time_t();

	return GetEventsOfInterval(dayStart, dayEnd, ignoreHidden);
}


BList*
QueryDBManager::GetEventsOfWeek(BDate date, bool ignoreHidden)
{
	date.AddDays(-date.DayOfWeek()+1);
	time_t weekStart = BDateTime(date, BTime(0, 0, 0)).Time_t();
	date.AddDays(6);
	time_t weekEnd = BDateTime(date, BTime(23, 59, 59)).Time_t();

	return GetEventsOfInterval(weekStart, weekEnd, ignoreHidden);
}


BList*
QueryDBManager::GetEventsOfMonth(BDate date, bool ignoreHidden)
{
	BDate startDate(date.Year(), date.Month(), 1);
	BDate endDate(date.Year(), date.Month(), date.DaysInMonth());
	time_t monthStart = BDateTime(startDate, BTime(0, 0, 0)).Time_t();
	time_t monthEnd = BDateTime(endDate, BTime(23, 59, 59)).Time_t();

	return GetEventsOfInterval(monthStart, monthEnd, ignoreHidden);
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

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		if (fTrashDir->Contains(BPath(&ref).Path()) == true)
			continue;
		event = _FileToEvent(&ref);
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
	query.PushOp(B_CONTAINS);
	query.PushOp(B_AND);

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		if (fTrashDir->Contains(BPath(&ref).Path()) == true)
			continue;
		event = _FileToEvent(&ref);
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
		if (fTrashDir->Contains(BPath(&ref).Path()) == true)
			continue;
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

	query.Fetch();

	entry_ref ref;
	entry_ref normalRef;
	entry_ref trashRef;

	while (query.GetNextRef(&ref) == B_OK) {
		if (fTrashDir->Contains(BPath(&ref).Path()) == true)
			trashRef = ref;
		else
			normalRef = ref;
	}

	if (normalRef.name == NULL)
		return trashRef;
	return normalRef;
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
QueryDBManager::GetEventsOfInterval(time_t start, time_t end,
	bool ignoreHidden)
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

	query.Fetch();
	entry_ref ref;

	BFile evFile;
	Event* event;

	while (query.GetNextRef(&ref) == B_OK) {
		event = _FileToEvent(&ref);
		uint16 status = event->GetStatus();
		bool hidden = (status & EVENT_DELETED) || (status & EVENT_HIDDEN);
		if (ignoreHidden == false || ignoreHidden == true && hidden == false)
			events->AddItem(event);
	}
	return events;
}


status_t
QueryDBManager::_GetFileOfId(const char* id, BFile* file, entry_ref* ref)
{
	BQuery query;
	query.SetVolume(&fQueryVolume);

	query.PushAttr("Calendar:ID");
	query.PushString(id);
	query.PushOp(B_EQ);

	entry_ref foundRef;
	status_t result = query.Fetch();

	if (result == B_OK && query.GetNextRef(&foundRef) == B_OK) {
		if (file != NULL)
			*file = BFile(&foundRef, B_READ_WRITE);
		if (ref != NULL)
			*ref = foundRef;
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
QueryDBManager::_FileToEvent(entry_ref* ref)
{
	BNode node(ref);
	BEntry entry(ref);
	if (node.InitCheck() != B_OK || entry.InitCheck() != B_OK)
		return NULL;

	BString name  = BString();
	BString catName = BString();
	BString idStr = BString();
	BString desc  = BString();
	BString place = BString();
	BString statusStr = BString();
	node.ReadAttrString("Event:Name", &name);
	node.ReadAttrString("Event:Category", &catName);
	node.ReadAttrString("Calendar:ID", &idStr);
	node.ReadAttrString("Event:Description", &desc);
	node.ReadAttrString("Event:Place", &place);
	node.ReadAttrString("Event:Status", &statusStr);

	time_t start	= time(NULL);
	time_t end		= time(NULL);
	time_t updated	= time(NULL);
	node.ReadAttr("Event:Start", B_TIME_TYPE, 0, &start, sizeof(time_t));
	node.ReadAttr("Event:End", B_TIME_TYPE, 0, &end, sizeof(time_t));
	node.ReadAttr("Event:Updated", B_TIME_TYPE, 0, &updated, sizeof(time_t));

	bool allDay = false;
	time_t dayStart	= BDateTime(BDate(start), BTime(0, 0, 0)).Time_t();
	time_t dayEnd	= BDateTime(BDate(end), BTime(23, 59, 0)).Time_t();
	if (dayStart <= start && start <= dayStart + 59
		&& dayEnd <= end && end <= dayEnd + 59)
		allDay = true;

	uint16 status = 0;
	if (statusStr.FindFirst("Notified") >= 0)
		status |= EVENT_NOTIFIED;
	if (statusStr.FindFirst("Cancelled") >= 0)
		status |= EVENT_CANCELLED;
	if (statusStr.FindFirst("Hidden") >= 0)
		status |= EVENT_HIDDEN;
	if (fTrashDir->Contains(&entry) == true)
		status |= EVENT_DELETED;

	return new Event(name.String(), place.String(), desc.String(), allDay,
					start, end, EnsureCategory(catName.String()), updated,
					status, idStr.String());
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

	size_t length = 0;
	uchar* icon = LoadRecoloredIcon("CATEGORY_ICON", &length,
		category->GetColor());
	if (icon != NULL) {
		file->WriteAttr("BEOS:ICON", B_VECTOR_ICON_TYPE, 0, icon, length);
		delete icon;
	}

	return true;
}


bool
QueryDBManager::_EventToFile(Event* event, BFile* file)
{
	if (BString(event->GetName()).CountChars() < 3)
		return false;

	if (_EventStatusSwitch(file->InitCheck()) != B_OK)
		return false;

	BString status;
	if (event->GetStatus() & EVENT_NOTIFIED)
		status << "Notified";
	else
		status << "Unnotified";
	if (event->GetStatus() & EVENT_CANCELLED) {
		if (status.IsEmpty() == false)
			status << ", ";
		status << "Cancelled";
	}
	if (event->GetStatus() & EVENT_HIDDEN) {
		if (status.IsEmpty() == false)
			status << ", ";
		status << "Hidden";
	}

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

	size_t length = 0;
	uchar* icon = LoadRecoloredIcon("EVENT_ICON", &length,
		event->GetCategory()->GetColor());
	if (icon != NULL) {
		file->WriteAttr("BEOS:ICON", B_VECTOR_ICON_TYPE, 0, icon, length);
		delete icon;
	}

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


status_t
QueryDBManager::_TrashStatusSwitch(status_t result)
{
	if (result == B_ENTRY_NOT_FOUND) {
		BAlert* alert = new BAlert(B_TRANSLATE("Moving to Trash"),
			B_TRANSLATE("Couldn't move the event to trash― it seems that the event "
			"couldn't be found or doesn't exist. You might want to "
			"use a Tracker query to find this event and manually "
			"delete it."),
			B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
	}
	else if (result != B_OK) {
		BAlert* alert = new BAlert(B_TRANSLATE("Moving to Trash"),
			B_TRANSLATE("Couldn't move the event to Trash― you might want to "
			"use a Tracker query to find this event and manually delete it."),
			B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
	}
	return result;
}


status_t
QueryDBManager::_RestoreStatusSwitch(status_t result)
{
	if (result == B_ENTRY_NOT_FOUND) {
		BAlert* alert = new BAlert(B_TRANSLATE("Restoring event"),
			B_TRANSLATE("Couldn't restore the deleted event― it seems that it "
			"couldn't be found or doesn't exist. You might want to "
			"look in the Trash to find this event and manually restore it."),
			B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
	}
	else if (result != B_OK) {
		BAlert* alert = new BAlert(B_TRANSLATE("Restoring event"),
			B_TRANSLATE("Couldn't restore the deleted event― you might want to "
			"look in the Trash to find this event and manually restore it."),
			B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
	}
	return result;
}


bool
QueryDBManager::_CategoryMimetype()
{
	BMessage info;
	uint8* iconData;
	size_t iconLength = 0;
	BMimeType mime("application/x-calendar-category" );
	if (mime.IsInstalled()
		&& mime.GetIcon(&iconData, &iconLength) != B_ENTRY_NOT_FOUND)
		return true;

	mime.GetAttrInfo(&info);

	mime.SetShortDescription("Calendar Category");
	mime.SetLongDescription("Category of calendar events");
	mime.SetPreferredApp("application/x-vnd.calendar");

	_AddAttribute(info, "Category:Name",	"Name", B_STRING_TYPE, true, 300);
	_AddAttribute(info, "Category:Color",	"Color", B_RGB_COLOR_TYPE, false, 100);
	_AddAttribute(info, "Calendar:ID",		"ID", B_STRING_TYPE, true, 100);

	const void* icon = LoadVectorIcon("CATEGORY_ICON", &iconLength);
	if (icon != NULL)
		mime.SetIcon((uint8*)icon, iconLength);

	return mime.SetAttrInfo( &info );
}


bool
QueryDBManager::_EventMimetype()
{
	BMessage info;
	uint8* iconData;
	size_t iconLength = 0;
	BMimeType mime("application/x-calendar-event" );
	if (mime.IsInstalled()
		&& mime.GetIcon(&iconData, &iconLength) != B_ENTRY_NOT_FOUND)
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

	const void* icon = LoadVectorIcon("EVENT_ICON", &iconLength);
	if (icon != NULL)
		mime.SetIcon((uint8*)icon, iconLength);

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


// "Cancelled" was previously synonymous with "Deleted"― so on first run after
// this change, all events with their status solely as "Cancelled" will be
// moved to Trash.
void
QueryDBManager::_MigrateCancellations(BPath cancelPath)
{
	BNode evNode;
	entry_ref ref;
	BDirectory cancelDir(cancelPath.Path());
	BString newStatus = "Unnotified";

	while (cancelDir.GetNextRef(&ref) == B_OK) {
		evNode.SetTo(&ref);
		if (evNode.InitCheck() == B_OK)
			evNode.WriteAttrString("Event:Status", &newStatus);
		BPath restorePath = _SettingsPath("events/");
		restorePath.Append(ref.name);
		RemoveEvent(ref, restorePath.Path());
	}
	BEntry(cancelPath.Path()).Remove();
}


status_t
QueryDBManager::_CreateUniqueFile(BDirectory* dir, BString name, BFile* newFile)
{
	return dir->CreateFile(_UniqueFilename(dir,
		name.ReplaceAllChars("/","∕", 0)).String(), newFile, true);
}


// Return a unique leaf-name similar to the given one. If a file of the given
// leaf already exists, then add a unique number to the end of it.
BString
QueryDBManager::_UniqueFilename(BDirectory* dir, BString leaf)
{
	if (dir->Contains(leaf.String()))
		return _UniqueFilename(dir, _IncrementSuffix(leaf));
	return leaf;
}


// Return an event name― Either one that is unique generally, or one allowed
// to be used by a specific event (by ID).
BString
QueryDBManager::_UniqueEventName(BString name, time_t startTime, const char* id)
{
	Event* event = GetEvent(name.String(), startTime);
	if (event == NULL || (id != NULL && strcmp(event->GetId(), id) == 0))
		return name;
	else
		return _UniqueEventName(_IncrementSuffix(name), startTime, id);
}


BString
QueryDBManager::_IncrementSuffix(BString string)
{
	int suffix = 1;
	char suffixStr[3];
	BStringList sections = BStringList();
	bool result = string.Split(" - ", true, sections);

	if (result && sections.CountStrings() > 1) {
		sscanf(sections.Last().String(), "%u", &suffix);
		if (suffix > 0)
			suffix++;

		sections.Remove(sections.CountStrings() - 1);
		string = sections.Join(" - ");
	}

	sprintf(suffixStr, "%u", suffix);
	string += " - ";
	string += suffixStr;
	return string;
}


BPath
QueryDBManager::_SettingsPath(const char* leaf)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(kDirectoryName);
	path.Append(leaf);
	return path;
}


BDirectory*
QueryDBManager::_EnsureDirectory(BPath path)
{
	BDirectory* dir = new BDirectory(path.Path());
	if (dir->InitCheck() == B_ENTRY_NOT_FOUND)
		dir->CreateDirectory(path.Path(), dir);
	return dir;
}
