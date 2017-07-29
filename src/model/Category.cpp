/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Category.h"

#include <stdio.h>


Category::Category(uint32 id, BString name, rgb_color color)
{
		fName = name;
        fColor = color;
		if (id == 0) {
			BString hashString;
			hashString.SetToFormat("%.6Xs", (color.red << 16)|
				(color.green << 8)|color.blue, name);
			fId = hashString.HashValue();
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
