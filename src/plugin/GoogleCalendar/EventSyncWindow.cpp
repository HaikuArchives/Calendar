/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventSyncWindow.h"

#include <Application.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <View.h>

#include "EventSync.h"


extern int32 SynchronizationLoop(void* data);


EventSyncWindow::EventSyncWindow()
	:
	BWindow(BRect(), "Google Caledar Sync", B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
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

	ResizeTo(300, 300);
	CenterOnScreen();

	fSynchronizationThread = -1;
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

		case kSyncFailedMessage:
			fStatusLabel->SetText("Last Sync Failed");
			break;

		case kSyncSuccessMessage:
			fStatusLabel->SetText("Last Sync Success");
			break;

		default:
			BWindow::MessageReceived(message);
		}
}


bool
EventSyncWindow::QuitRequested()
{
	be_app->PostMessage(kEventSyncWindowQuitting);
	return true;
}


void
EventSyncWindow::_Sync()
{
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
EventSyncWindow::_SetStatus()
{


}
