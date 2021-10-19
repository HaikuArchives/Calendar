/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ColorConverter.h"

#include <stdlib.h>


BString
RGBToHex(rgb_color color)
{

	BString hexColor;
	hexColor.SetToFormat("%.6X", (color.red << 16)
		|(color.green << 8)|color.blue);
	return hexColor;
}


rgb_color
HexToRGB(const BString& color)
{
	const char* hexColor = color.String();
    int rgb = (int)strtol(hexColor, NULL, 16);
	uint8 r = rgb >> 16;
	uint8 g = rgb >> 8 & 0xFF;
	uint8 b = rgb & 0xFF;

	return (rgb_color){r, g, b};
}
