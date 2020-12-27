/*
 
TimeInfo.h

a-book -- a small calendar application with reminder

Copyright (C) 2002 Maurice Michalski, http://fetal.de, http://maurice-michalski.de

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
*/
#ifndef _TIMEINFO_H_
#define _TIMEINFO_H_

#include <unistd.h>
#include <KernelKit.h>

static const char *dayofweek[8]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
static const char *dayofweek_short[8]={"SU", "MO", "TU", "WE", "TH", "FR", "SA", "SU"};

static const char *monthname[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
static const char *monthname_short[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char *monthname_shortest[12]={"J", "F", "M", "A", "M", "J", "J", "A", "S", "O", "N", "D"};

int GetDay();			// 1..31
int GetMonth();		// 0..11
int GetYear();		// ex: 2002
int GetDayOfWeek();	// 0..6 (0=Sunday)
int GetDayOfYear();		// 0..365

int GetSecond();		// 0..61
int GetMinute();		// 0..59
int GetHour();		// 0..23

int GetFirstDOW();		// 0..6 (0=Sunday)
int GetDOW(int day, int month, int year);	// 0..6 (0=Sunday)
int GetDOY(int day, int month, int year);	// 0..365
int GetDaysInMonth(int month, int year);	// 28..31
int GetDaysInYear(int year);
int GetDaysFrom1984To(int day, int month, int year);

int MinutesBefore(int day, int month, int year, int hour, int minute);
void AddMinutesToDate(int &day, int &month, int &year, int &hour, int &minute, int32 minutes_to_add);


#endif
