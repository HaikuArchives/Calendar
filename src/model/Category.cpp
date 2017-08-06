/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Category.h"

#include <stdio.h>
#include <stdlib.h>

#include "ColorConverter.h"


Category::Category(uint32 id, BString name, rgb_color color)
{
		fName = name;
        fColor = color;
		if (id == 0) {
			fId = name.HashValue();
		}
		else
			fId = id;
}


Category::Category(uint32 id, BString name, BString color)
{
		fName = name;
		fColor = HexToRGB(color);

		if (id == 0) {
			fId = name.HashValue();
		}
		else
			fId = id;
}


Category::Category(Category &category)
{
	fName = category.GetName();
	fColor = category.GetColor();
	fId = category.GetId();
}


BString
Category::GetName()
{
	return fName;
}


rgb_color
Category::GetColor()
{
	return fColor;
}


BString
Category::GetHexColor()
{
	return RGBToHex(fColor);
}


uint32
Category::GetId()
{
	return fId;
}


bool
Category::Equals(Category &c)
{
	return (fId == c.GetId());
}
