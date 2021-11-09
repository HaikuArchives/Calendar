/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef RESOURCE_LOADER_H
#define RESOURCE_LAODER_H

#include <Bitmap.h>


BBitmap*	LoadVectorIcon(const char* name, int32 iconSize = 32,
				int32 cropSize = 22);
void*		LoadVectorIcon(const char* name, size_t* length);
uchar*		LoadRecoloredIcon(const char* name, size_t* length,
				rgb_color newColor);

void		RecolorIcon(uchar* icon, rgb_color oldColor, rgb_color newColor,
				size_t length);

#endif
