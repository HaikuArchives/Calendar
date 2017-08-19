/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_SYNC_WINDOW_H
#define _EVENT_SYNC_WINDOW_H

#include <Path.h>
#include <String.h>
#include <Window.h>


class BButton;
class BStringView;
class BView;


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
	void			_Sync();
	void			_LoadSyncData();
	void			_SaveSyncData(bool status);
	void			_StartSynchronizationThread();
	void			_StopSynchronizationThread();
	void			_SetStatusLabel(bool status, time_t syncTime);

	static const uint32	kSyncPressed = 1000;

	BStringView*	fStatusLabel;
	BButton*		fSyncButton;
	BView*			fMainView;
	BPath			fSyncDataFile;

	thread_id		fSynchronizationThread;
	BMessage*		fThreadMessage;
};

#endif
