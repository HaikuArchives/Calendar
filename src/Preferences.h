/*
 * Copyright 2017 Akshay Agarawl, agarwal.akshay.akshay8@gmail.com
 * Copyright 2014-2017 Kacper Kasper, kacperkasper@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H


#include <string>

#include <Path.h>
#include <String.h>


class Preferences {
public:
	void					Load(const char* filename);
	void					Save(const char* filename);

	Preferences&			operator =(const Preferences& p);

	BPath					fSettingsPath;

	int32					fStartOfWeekOffset;
	bool					fHeaderVisible;
	bool					fShowbottomVisible;
	bool					fUseRightSide;
	BString				fDefaultCategory;
	BRect					fMainWindowRect;
	BRect					fEventWindowRect;
};


#endif
