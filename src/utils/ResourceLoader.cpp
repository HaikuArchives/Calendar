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


BBitmap* LoadVectorIcon(const char* name, int32 iconSize, int32 cropSize)
{
	BResources* res = BApplication::AppResources();
	size_t length = 0;
	const void* data = res->LoadResource(B_VECTOR_ICON_TYPE, name, &length);
	BBitmap* temp = new BBitmap(BRect(0, 0, iconSize, iconSize), B_RGBA32);
	BBitmap* dest = new BBitmap(BRect(0, 0, cropSize, cropSize), B_RGBA32);
	if (data != NULL &&
		BIconUtils::GetVectorIcon((uint8*)data, length, temp) == B_OK)
			if (dest->ImportBits(temp, BPoint(0, 0),
				BPoint(0, 0), cropSize, cropSize) == B_OK)

				return dest;

	delete temp;
	delete dest;
	return NULL;
}
