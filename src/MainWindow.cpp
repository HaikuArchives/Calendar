/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <LayoutBuilder.h>
#include <Application.h>

#include "MainWindow.h"


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
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.Add(fMenuBar)
		.AddGlue()
		.Add(fMainView)
		.AddGlue()
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
