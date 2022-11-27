![Calendar Icon](documentation/images/calendar_icon_64.png) **Calendar** for [Haiku](https://www.haiku-os.org/).

* * *

Calendar is a native Haiku application to manage your appointments.    
You can create, edit and delete events, and cancel or hide them, categorize events and show them in a day, week or month view.

![screenshot](documentation/images/main_window.png)

Requirements
-------
To build Calendar you need the sqlite3 development library: 

  For 32 bit version of Haiku   -- ```$ pkgman install sqlite_x86_devel```
  
  For 64 bit version of Haiku   -- ```$ pkgman install sqlite_devel```  
   

Note
-------
When building on 32bit Haiku, you need to change the build environment to use gcc11 with `setarch x86` before invoking `make`.


For more information, please see the [Calendar documentation](http://htmlpreview.github.io/?https://github.com/HaikuArchives/Calendar/master/documentation/Documentation.html).
