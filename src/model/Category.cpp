/*
 * Copyright 2017 Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Category.h"


Category::Category(uint32 id, BString name, BString color)
{
		fName = name;
        fColor = color;
		if (id == 0) {
			BString hashString;
			hashString.Append(name);
			hashString.Append(color);
			hashString+=color;
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


BString
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
