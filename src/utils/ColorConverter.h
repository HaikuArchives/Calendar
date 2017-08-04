/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _COLOR_CONVERTER_H_
#define _COLOR_CONVERTER_H_


#include <GraphicsDefs.h>
#include <String.h>


BString RGBToHex(rgb_color color);

rgb_color HexToRGB(const BString& color);

#endif  //_COLOR_CONVERTER_H_
