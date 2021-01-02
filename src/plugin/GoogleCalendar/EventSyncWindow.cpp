/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventSyncWindow.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <DateFormat.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <View.h>
#include <String.h>
#include <Key.h>
#include <KeyStore.h>

#include "App.h"
#include "EventSync.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GoogleEventSyncWindow"

extern int32 SynchronizationLoop(void* data);


EventSyncWindow::EventSyncWindow()
	:
	BWindow(BRect(), B_TRANSLATE("Google Calendar Sync"), B_TITLED_WINDOW,
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

	fLogMessage = new BTextView("Log");
	//fLogMessage->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	//fLogMessage->SetLowColor(fDescriptionView->ViewColor());
	//fLogMessage->MakeEditable(false);

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

		case kRemovePressed:
			_RemoveKey();
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

	fStatusLabel = new BStringView("Status Label","");
	fSyncButton = new BButton(NULL, "Sync", new BMessage(kSyncPressed));
	fRemoveButton = new BButton(NULL, "Remove Password", new BMessage(kRemovePressed));

	BLayoutBuilder::Group<>(fMainView, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.Add(fStatusLabel)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fSyncButton)
			.Add(fRemoveButton)
		.End()
		.Add(fLogMessage)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fMainView)
	.End();
}


void
EventSyncWindow::_Sync()
{
	_SetStatusMessage(B_TRANSLATE("Please wait while we sync..."));
	_StartSynchronizationThread();
}

void
EventSyncWindow::_SetStatusMessage(const char* str)
{
	BString bstr;
	bstr << fLogMessage->Text() << str <<"\n";
	fLogMessage->SetText(bstr.String());
}


void
EventSyncWindow::_StartSynchronizationThread()
{
	if (fSynchronizationThread < 0) {
		_SetStatusMessage(B_TRANSLATE("StartSynchronizationThread"));
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
		_SetStatusMessage(B_TRANSLATE("StopSynchronizationThread"));
		kill_thread(fSynchronizationThread);
		delete fThreadMessage;
		fSynchronizationThread = -1;
	}
}


void
EventSyncWindow::_SetStatusLabel(bool status, time_t syncTime)
{
	BString statusText(B_TRANSLATE("Last Sync: %status% at %time%."));
	BString statusString = (status)? B_TRANSLATE("Success") : B_TRANSLATE("Failed");


	BString timeString;
	BDateTimeFormat().Format(timeString, syncTime, B_SHORT_DATE_FORMAT,
		B_SHORT_TIME_FORMAT);

	statusText.ReplaceAll("%status%", statusString);
	statusText.ReplaceAll("%time%", timeString);

	fStatusLabel->SetText(statusText);
}


void
EventSyncWindow::_LoadSyncData()
{
	BMessage* message = new BMessage();
	BFile* file = new BFile(fSyncDataFile.Path(), B_READ_ONLY);

	if (file->InitCheck() == B_OK) {
		_SetStatusMessage(B_TRANSLATE("LoadSyncData"));
		if (message->Unflatten(file) == B_OK) {
			_SetStatusMessage(B_TRANSLATE("Unflatten file"));
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
		_SetStatusMessage(B_TRANSLATE("There was an error in loading the last sync data."));
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
		_SetStatusMessage(B_TRANSLATE("SaveSyncData"));
		message->AddBool("syncStatus", status);
		message->AddData("syncTime", B_RAW_TYPE, (void*)&syncTime,
			sizeof(time_t));
		message->Flatten(file);
	}
	else
	{
		_SetStatusMessage(B_TRANSLATE("There was an error in saving the recent sync data."));
	}

	_SetStatusLabel(status, syncTime);

	delete file;
	delete message;
}


void
EventSyncWindow::_RemoveKey()
{
	BPasswordKey key;
	BKeyStore keyStore;
	if(keyStore.GetKey(kAppName, B_KEY_TYPE_PASSWORD, "refresh_token", key) == B_OK) {
		status_t result = keyStore.RemoveKey(kAppName, key);
		if (result != B_OK) {
			_SetStatusMessage(B_TRANSLATE("Password/Token removed"));
		}
		else
		{
			_SetStatusMessage(B_TRANSLATE("Error remove Password/Token"));
		}
	}
	else
	{
		_SetStatusMessage(B_TRANSLATE("No Password/Token to remove"));
	}
}
