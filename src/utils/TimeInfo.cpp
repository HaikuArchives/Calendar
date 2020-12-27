/*
 
TimeInfo.cpp

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
#include "TimeInfo.h"
#include <time.h>

const struct tm gettime() {
	time_t	t=time(NULL);
	return *localtime(&t);
}

int GetDay() {
	return gettime().tm_mday;
}

int GetMonth() {
	return gettime().tm_mon;
}

int GetYear() {
	return gettime().tm_year+1900;
}

int GetDayOfWeek() {
	return gettime().tm_wday;
}

int GetDayOfYear() {
	return gettime().tm_yday;
}

int GetSecond() {
	return gettime().tm_sec;
}

int GetMinute() {
	return gettime().tm_min;
}

int GetHour() {
	return gettime().tm_hour;
}

int GetFirstDOW() {
	int dow=GetDayOfWeek()-(GetDayOfYear() % 7)+21;
	
	while (dow>6) dow-=7;
	return dow;
}

int GetDOW(int day, int month, int year) {
	int dayssince1984=GetDaysFrom1984To(day, month, year);
	int dow=dayssince1984 % 7;
	
	while (dow<0) dow+=7;
	while (dow>6) dow-=7;
	return dow;
}

int GetDaysInMonth(int month, int year) {
	switch(month) {
		case 0: return 0;
		case 1: return 31;
		case 2: {
			if (((year % 4)==0) && ((year % 100)!=0))
				return 29;
			else
				return 28;
		}
		case 3: return 31;
		case 4: return 30;
		case 5: return 31;
		case 6: return 30;
		case 7: return 31;
		case 8: return 31;
		case 9: return 30;
		case 10: return 31;
		case 11: return 30;
		case 12: return 31;
		default: return 31;
	}
}

int GetDaysInYear(int year) {
	int days=0;
	
	for (int i=1; i<13; i++)
		days+=GetDaysInMonth(i, year);
	
	return days;
}

int GetDaysFrom1984To(int day, int month, int year) {
	int y;
	int32 days=0;
	
	if (year<1984) return -1;
	
	for (y=1984; y<year;y++) {
		days+=GetDaysInYear(y);
	}
	days+=GetDOY(day, month, year);
	
	return days;
}

int GetDOY(int day, int month, int year) {
	int m, doy=0;
	
	for (m=0; m<month; m++) {
		doy+=GetDaysInMonth(m, year);
	}
	
	doy+=day;
	
	return doy;
}

// negative values mean day/month/year/hour/minute is in the past
int	MinutesBefore(int day, int month, int year, int hour, int minute) {
	int	td=GetDay(),
		tm=GetMonth(),
		ty=GetYear(),
		th=GetHour(),
		tmin=GetMinute();
		
	int32	m1, m2;
	
	m1=(int32)GetDaysFrom1984To(td, tm, ty)*24*60;
	m1+=th*60+tmin;
	// m1: minutes since 01/01/1984 (until now).
	
	m2=(int32)GetDaysFrom1984To(day, month, year)*24*60;
	m2+=hour*60+minute;
	// m2: minutes since 01/01/1984 until month/day/year.
	
	return m2-m1;	
}


// month: 1..12
// minute: 1..59
// hour: 0..23
void	AddMinutesToDate(int &day, int &month, int &year, int &hour, int &minute, int32 minutes_to_add) {
	
	minute+=minutes_to_add;
	if (minute>=60) {
		hour++;
		minute-=60;
		if (hour>=24) {
			day++;
			hour-=24;
			if (day>=GetDaysInMonth(month, year)) {
				month++;
				day=1;
				if (month>12) {
					year++;
					month=1;
				}
			}
		}
	}
}

