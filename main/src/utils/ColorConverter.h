/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _COLOR_CONVERTER_H_
#define _COLOR_CONVERTER_H_


#include <GraphicsDefs.h>
#include <String.h>


BString RGBToHex(rgb_color color);
rgb_color HexToRGB(const BString& color);

rgb_color TintColor(rgb_color color, rgb_color base, int severity);


#endif //_COLOR_CONVERTER_H_
