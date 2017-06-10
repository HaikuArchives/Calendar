/*
 * Copyright 2017 Akshay Agarwal<agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PREFWIN_H_
#define _PREFWIN_H_

#include <CheckBox.h>
#include <DateFormat.h>
#include <Menu.h>
#include <MenuField.h>
#include <StringView.h>
#include <View.h>
#include <Window.h>


class PreferenceWindow : public BWindow {
public:
							PreferenceWindow();

		void				MessageReceived(BMessage *message);
		bool				QuitRequested();


		bool				IsWeekHeaderEnabled();
		void				SetWeekHeader(bool);
		int					GetStartOfWeek();
		void				SetStartOfWeek(int);

private:

		static const int	kStartOfWeekChangeMessage 	= 1000;
		static const int 	kShowWeekChangeMessage		= 1001;

		BView*				fMainView;
		BCheckBox*			fWeekNumberHeaderCB;
		BStringView*		fPrefCategoryLabel;
		BStringView*		fStartOfWeekLabel;
		BMenu*				fDayOfWeekMenu;
		BMenuField*			fDayOfWeekMenuField;
};

#endif
