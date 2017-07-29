/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORY_H
#define CATEGORY_H

#include <GraphicsDefs.h>
#include <String.h>

class Category {
public:
		Category(uint32 id, BString name, rgb_color color);
		Category(Category& category);

		BString GetName();
		rgb_color GetColor();
		uint32 GetId();

		bool Equals(Category &c);

private:
		uint32			fId;
		BString 		fName;
		rgb_color 		fColor;

};

#endif
