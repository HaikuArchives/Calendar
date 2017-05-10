/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <MenuBar.h>
#include <MenuItem.h>
#include <View.h>
#include <Window.h>


class MainWindow: public BWindow
{
public:
			MainWindow();
	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	static const int kMenuAppQuit = 1000;
	
	BView*		fMainView;
	BMenuBar*	fMenuBar;
	BMenu*		fAppMenu;
};

#endif
