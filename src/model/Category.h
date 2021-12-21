/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CATEGORY_H
#define CATEGORY_H

#include <GraphicsDefs.h>
#include <ObjectList.h>
#include <String.h>


class Category {
public:
		Category(BString name, rgb_color color,
			const char* id = NULL);
		Category(BString name, BString color,
			const char* id = NULL);
		Category(Category& category);

		BString GetName();
		rgb_color GetColor();
		BString GetHexColor();
		const char* GetId();

		bool Equals(Category &c);

private:
		BString			fId;
		BString 		fName;
		rgb_color 		fColor;

};


typedef BObjectList<Category> CategoryList;

#endif
