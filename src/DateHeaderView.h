/*
 * Copyright 2017 Akshay Agarwal<agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _DATEHEADERVIEW_H_
#define _DATEHEADERVIEW_H_

#include <StringView.h>
#include <View.h>


class DateHeaderView: public BView {
public:
							DateHeaderView();
		
private:
		BStringView*		fDayLabel;
		BStringView*		fDayOfWeekLabel;
		BStringView*		fMonthYearLabel;
};
	
#endif
