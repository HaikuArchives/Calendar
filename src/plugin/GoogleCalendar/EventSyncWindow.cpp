/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventSyncWindow.h"

#include <Application.h>
#include <Button.h>
#include <DateFormat.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <View.h>

#include "App.h"
#include "EventSync.h"


extern int32 SynchronizationLoop(void* data);


EventSyncWindow::EventSyncWindow()
	:
	BWindow(BRect(), "Google Calendar Sync", B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
	fSynchronizationThread(-1)
{
	BPath syncDataPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &syncDataPath);
	syncDataPath.Append(kAppName);
	BDirectory syncDataDir(syncDataPath.Path());
	if(syncDataDir.InitCheck() == B_ENTRY_NOT_FOUND) {
		syncDataDir.CreateDirectory(syncDataPath.Path(), &syncDataDir);
	}

	fSyncDataFile.SetTo(&syncDataDir, "sync");

	_InitInterface();
	ResizeTo(240, 240);
	CenterOnScreen();

	_LoadSyncData();
}


EventSyncWindow::~EventSyncWindow()
{
}


void
EventSyncWindow::MessageReceived(BMessage* message)
{

	switch(message->what) {

		case kSyncPressed:
			_Sync();
			break;

		case kSyncStatusMessage:
		{
			bool status;
			message->FindBool("status", &status);
			_SaveSyncData(status);
			be_app->WindowAt(0)->PostMessage(kSynchronizationComplete);
			_StopSynchronizationThread();
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
EventSyncWindow::QuitRequested()
{
	if (fSynchronizationThread > 0)
		return false;

	be_app->PostMessage(kEventSyncWindowQuitting);
	return true;
}


void
EventSyncWindow::_InitInterface()
{
	BView* fMainView = new BView("MainView", B_WILL_DRAW);
	fMainView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fStatusLabel = new BStringView("Status Label",
		"No Data Available");
	fSyncButton = new BButton(NULL, "Sync", new BMessage(kSyncPressed));

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.Add(fStatusLabel)
		.Add(fSyncButton)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fMainView)
	.End();
}


void
EventSyncWindow::_Sync()
{
	fStatusLabel->SetText("Please wait while we sync...");
	_StartSynchronizationThread();
}


void
EventSyncWindow::_StartSynchronizationThread()
{
	if (fSynchronizationThread < 0) {
		fThreadMessage = new BMessage();
		fThreadMessage->AddPointer("handler", this);
		fSynchronizationThread = spawn_thread(SynchronizationLoop,
			"Synchronization Thread", B_NORMAL_PRIORITY, fThreadMessage);
		resume_thread(fSynchronizationThread);
	}
}


void
EventSyncWindow::_StopSynchronizationThread()
{
	if (fSynchronizationThread > 0) {
		kill_thread(fSynchronizationThread);
		delete fThreadMessage;
		fSynchronizationThread = -1;
	}
}


void
EventSyncWindow::_SetStatusLabel(bool status, time_t syncTime)
{
	BString statusText;
	BString statusString = (status)? "Success" : "Failed";

	statusText << "Last Sync: " << statusString;

	BString timeString;
	BDateTimeFormat().Format(timeString, syncTime, B_SHORT_DATE_FORMAT,
		B_SHORT_TIME_FORMAT);
	statusText << " at " << timeString << ".";

	fStatusLabel->SetText(statusText);
}


void
EventSyncWindow::_LoadSyncData()
{
	BMessage* message = new BMessage();
	BFile* file = new BFile(fSyncDataFile.Path(), B_READ_ONLY);


	if (file->InitCheck() == B_OK) {
		if (message->Unflatten(file) == B_OK) {
			bool lastSyncStatus;
			message->FindBool("syncStatus", &lastSyncStatus);
			time_t* lastSyncTime;
			ssize_t size;
			message->FindData("syncTime", B_RAW_TYPE,
				(const void**)&lastSyncTime, &size);
			_SetStatusLabel(lastSyncStatus, *lastSyncTime);
		}
	}

	else
	{
		BAlert* alert  = new BAlert("Error",
			"There was an error in loading the last sync data.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	}


	delete file;
	delete message;

}

void
EventSyncWindow::_SaveSyncData(bool status)
{
	BMessage* message = new BMessage();
	BFile* file = new BFile(fSyncDataFile.Path(),
		B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);

	time_t syncTime = time(NULL);

	if (file->InitCheck() == B_OK) {
		message->AddBool("syncStatus", status);
		message->AddData("syncTime", B_RAW_TYPE, (void*)&syncTime,
			sizeof(time_t));
		message->Flatten(file);
	}

	else
	{
		BAlert* alert  = new BAlert("Error",
			"There was an error in saving the recent sync data.",
			NULL, "OK",NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);

	}

	_SetStatusLabel(status, syncTime);

	delete file;
	delete message;
}
