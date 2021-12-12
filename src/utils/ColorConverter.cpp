/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ColorConverter.h"

#include <stdlib.h>

#include <InterfaceDefs.h>


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


rgb_color
TintColor(rgb_color color, rgb_color base, int severity)
{
	bool dark = false;
	if (base.Brightness() < 127)
		dark = true;

	switch (severity) {
		case 0:
			return color;
		case 1:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_1_TINT + 0.1f);
			else
				return tint_color(color, B_DARKEN_1_TINT);
		case 2:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_1_TINT);
			else
				return tint_color(color, B_DARKEN_2_TINT);
		case 3: // intentional fallthrough
		case 4:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_2_TINT);
			else
				return tint_color(color, B_DARKEN_3_TINT);
		default:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_2_TINT - 0.1f);
			else
				return tint_color(color, B_DARKEN_4_TINT);
	}
}
