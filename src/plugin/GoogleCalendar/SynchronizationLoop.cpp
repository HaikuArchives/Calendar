/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EventSync.h"

#include <Handler.h>


int32
SynchronizationLoop(void* data)
{
	BMessage* message = (BMessage*)data;
	BHandler* handler;

	message->FindPointer("handler", (void**)(&handler));

	EventSync* sync = new EventSync(handler);
	sync->Login();
}
