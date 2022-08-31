/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EventSync.h"

#include <Handler.h>


int32
SynchronizationLoop(void* data)
{
	BMessage* message = (BMessage*) data;
	BHandler* handler;

	message->FindPointer("handler", (void**) (&handler));

	status_t status;
	status = B_ERROR;

	EventSync* sync = new EventSync();
	status = sync->Sync();

	BMessage statusMessage(kSyncStatusMessage);
	BMessenger msgr(handler);

	if (status != B_OK)
		statusMessage.AddBool("status", false);
	else
		statusMessage.AddBool("status", true);

	msgr.SendMessage(&statusMessage);

	return B_OK;
}
