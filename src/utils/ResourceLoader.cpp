/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ResourceLoader.h"

#include <Application.h>
#include <IconUtils.h>
#include <Mime.h>
#include <Resources.h>
#include <TranslationUtils.h>


BBitmap* LoadVectorIcon(const char* name, int32 size)
{
	BResources* res = BApplication::AppResources();
	size_t length = 0;
	const void* data = res->LoadResource(B_VECTOR_ICON_TYPE, name, &length);
	BBitmap* dest = new BBitmap(BRect(0, 0, size, size), B_RGBA32);
	if (data != NULL &&
		BIconUtils::GetVectorIcon((uint8*)data, length, dest) == B_OK)
		return dest;
	delete dest;
	return NULL;
}
