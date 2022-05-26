/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Category.h"

#include <stdio.h>
#include <stdlib.h>

#include <Uuid.h>

#include "ColorConverter.h"


Category::Category(BString name, rgb_color color, const char* id /*= NULL*/)
{
	fName = name;
	fColor = color;

	if (id == NULL)
		fId = BUuid().SetToRandom().ToString();
	else
		fId = id;
}


Category::Category(BString name, BString color, const char* id /*= NULL*/)
{
	fName = name;
	fColor = HexToRGB(color);

	if (id == NULL)
		fId = BUuid().SetToRandom().ToString();
	else
		fId = id;
}


Category::Category(Category& category)
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


const char*
Category::GetId()
{
	return fId.String();
}


bool
Category::Equals(Category& c)
{
	return (fId == c.GetId());
}
