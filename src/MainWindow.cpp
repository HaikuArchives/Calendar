/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MainWindow.h"

#include <Application.h>
#include <LayoutBuilder.h>
#include "ResourceLoader.h"


MainWindow::MainWindow()
	:
	BWindow(BRect(), "Calendar", B_TITLED_WINDOW, 
		B_AUTO_UPDATE_SIZE_LIMITS)
{
	ResizeTo(640, 360);
	CenterOnScreen();

	fMainView = new BView("MainView", B_WILL_DRAW);
	rgb_color bgcolor = ui_color(B_PANEL_BACKGROUND_COLOR);  
	fMainView->SetViewColor(bgcolor);
	
	fMenuBar = new BMenuBar("MenuBar");
	fAppMenu = new BMenu("App");

	BMenuItem* item = new BMenuItem("About", new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);

	fAppMenu->AddItem(item);
	fAppMenu->AddSeparatorItem();
	fAppMenu->AddItem(new BMenuItem("Quit", new BMessage(kMenuAppQuit), 'Q', B_COMMAND_KEY));
	
	fMenuBar->AddItem(fAppMenu);
	
	fToolBar = new BToolBar();
	fToolBar->AddAction(new BMessage(kShowToday), this, LoadVectorIcon("CALENDAR_ICON"),
		"Today", NULL, true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kDayView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Day View", NULL, true);
	fToolBar->AddAction(new BMessage(kMonthView), this, LoadVectorIcon("CALENDAR_ICON"),
		"Month View", NULL, true);
	fToolBar->AddSeparator();
	fToolBar->AddAction(new BMessage(kAddEvent), this, LoadVectorIcon("ADD_EVENT"),
		"Add Event", NULL, true);
	fToolBar->AddGlue();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.Add(fMenuBar)
		.Add(fToolBar)
		.AddGroup(B_VERTICAL)
			.SetInsets(0.0f)
			.Add(fMainView)
			.End()
	.End();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {

		case kMenuAppQuit:
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
