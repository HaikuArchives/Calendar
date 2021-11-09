/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyight 2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ResourceLoader.h"

#include <Application.h>
#include <Cursor.h>
#include <IconUtils.h>
#include <Mime.h>
#include <Resources.h>
#include <TranslatorFormats.h>
#include <TranslationUtils.h>

// The "greenscreen" color used by recoloured icons
const rgb_color SUBSTITUTION_COLOR = { 12, 135, 03 };


BBitmap*
LoadVectorIcon(const char* name, int32 iconSize, int32 cropSize)
{
	size_t length = 0;
	const void* data = LoadVectorIcon(name, &length);
	BBitmap* temp = new BBitmap(BRect(0, 0, iconSize - 1, iconSize - 1),
		B_BITMAP_NO_SERVER_LINK, B_RGBA32);
	BBitmap* dest = new BBitmap(BRect(0, 0, cropSize - 1, cropSize - 1),
		B_RGBA32);
	if (data != NULL
		&& BIconUtils::GetVectorIcon((uint8*)data, length, temp)
			== B_OK
		&& dest->ImportBits(temp, BPoint(0, 0), BPoint(0, 0),
			cropSize, cropSize) == B_OK)  {
			delete temp;
			return dest;
	}

	delete temp;
	delete dest;
	return NULL;
}


void*
LoadVectorIcon(const char* name, size_t* length)
{
	BResources* res = BApplication::AppResources();
	return (void*)res->LoadResource(B_VECTOR_ICON_TYPE, name, length);
}


uchar*
LoadRecoloredIcon(const char* name, size_t* length, rgb_color newColor)
{
	uchar* newIcon = NULL;
	void* icon = LoadVectorIcon(name, length);
	if (icon != NULL) {
		newIcon = new uchar[*length];
		memcpy(newIcon, icon, *length);
		RecolorIcon(newIcon, SUBSTITUTION_COLOR, newColor, *length);
	}
	return newIcon;
}


void
RecolorIcon(uchar* icon, rgb_color oldColor, rgb_color newColor, size_t length)
{
	for (int i = 0; i < length; i++)
		if (icon[i] == oldColor.red && icon[i+1] == oldColor.green
			&& icon[i+2] == oldColor.blue)
		{
			icon[i] = newColor.red;
			icon[i + 1] = newColor.green;
			icon[i + 2] = newColor.blue;
		}
}
