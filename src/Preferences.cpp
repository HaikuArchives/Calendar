/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2014-2017 Kacper Kasper, kacperkasper@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Preferences.h"

#include <Alert.h>
#include <File.h>
#include <Message.h>


void
Preferences::Load(const char* filename)
{
	BFile* file = new BFile(filename, B_READ_ONLY);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert("Configuration file",
				"Couldn't open configuration file because the path is not specified. It usually "
				"means that the programmer made a mistake. There is nothing you can do about it. "
				"Your personal settings will not be loaded. Sorry.", "OK", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert("Configuration file",
				"Couldn't open configuration file because permission was denied. It usually "
				"means that you don't have read permissions to your settings directory. "
				"If you want to have your personal settings loaded, check your OS documentation "
				"to find out which directory it is and try changing its permissions.", "OK",
				NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert("Configuration file",
				"There is not enough memory available on your system to load the configuration "
				"file. If you want to have your personal settings loaded, try closing few "
				"applications and restart Calendar.", "OK", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		default:
			break;
	}

	BMessage storage;
	if(result == B_OK)
		storage.Unflatten(file);
	fStartOfWeekOffset = storage.GetInt32("startOfWeekOffset", 0);
	fHeaderVisible = storage.GetBool("headerVisible", false);

	delete file;
}


void
Preferences::Save(const char* filename)
{
	BFile* file = new BFile(filename, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert("Configuration file",
				"Couldn't open configuration file because the path is not specified. It usually "
				"means that the programmer made a mistake. There is nothing you can do about it. "
				"Your personal settings will not be saved. Sorry.", "OK", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert("Configuration file",
				"Couldn't open configuration file because permission was denied. It usually "
				"means that you don't have write permissions to your settings directory. "
				"If you want to have your personal settings loaded, check your OS documentation "
				"to find out which directory it is and try changing its permissions.", "OK",
				NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert("Configuration file",
				"There is not enough memory available on your system to save the configuration "
				"file. If you want to have your personal settings saved, try closing few "
				"applications and try again.", "OK", NULL, NULL,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			alert->Go();
			break;
		}
		default:
			break;
	}

	BMessage storage;
	storage.AddInt32("startOfWeekOffset", fStartOfWeekOffset);
	storage.AddBool("headerVisible", fHeaderVisible);

	storage.Flatten(file);

	delete file;
}


Preferences&
Preferences::operator =(Preferences p)
{
	fSettingsPath = p.fSettingsPath;
	fStartOfWeekOffset = p.fStartOfWeekOffset;
	fHeaderVisible = p.fHeaderVisible;
}
