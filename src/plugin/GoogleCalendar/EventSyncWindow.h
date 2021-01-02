/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_SYNC_WINDOW_H
#define _EVENT_SYNC_WINDOW_H

#include <Path.h>
#include <String.h>
#include <Window.h>
#include <TextView.h>


class BButton;
class BStringView;
class BView;
class BTextView;


static const uint32 kEventSyncWindowQuitting = 'kswq';
static const uint32 kSynchronizationComplete = 'kesc';


class EventSyncWindow: public BWindow {
public:
					EventSyncWindow();
					~EventSyncWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	void			_InitInterface();
	void			_LoadSyncData();
	void			_SaveSyncData(bool status);
	void			_StartSynchronizationThread();
	void			_StopSynchronizationThread();
	void			_SetStatusLabel(bool status, time_t syncTime);
	void			_SetStatusMessage(const char* str);
	void			_RemoveKey();

	static const uint32	kSyncPressed = 1000;
	static const uint32	kRemovePressed = 1001;

	BStringView*	fStatusLabel;
	BButton*		fSyncButton;
	BButton*		fRemoveButton;
	BView*			fMainView;
	BPath			fSyncDataFile;
	BTextView*		fLogMessage;

	thread_id		fSynchronizationThread;
	BMessage*		fThreadMessage;
};

#endif
