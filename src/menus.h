// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 1996 Matthias Ettrich
*           and the LyX Team.
*
* ====================================================== */

#ifndef MENUS_H
#define MENUS_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include "lyx.h"
#include "LString.h"

class BufferView;
class LyXView;


///
class Menus {
public:
	///
	Menus(LyXView *view, int air);
	///
	void showMenus();
	///
	void hideMenus();
	///
	void openByName(string const &menuName);
	///
	static void ShowFileMenu(FL_OBJECT *ob, long);
	///
	static void ShowFileMenu2(FL_OBJECT *ob, long);
	///
	static void ShowEditMenu(FL_OBJECT *ob, long data);
	///
	static void ShowLayoutMenu(FL_OBJECT *ob, long data);
	///
	static void ShowInsertMenu(FL_OBJECT *ob, long data);
	///
	static void ShowMathMenu(FL_OBJECT *ob, long);
	///
	static void ShowOptionsMenu(FL_OBJECT *ob, long men);
	///
	static void ShowBufferMenu(FL_OBJECT *ob, long);
	///
	static void ShowHelpMenu(FL_OBJECT *ob, long);
private:
	///
	void create_menus(int air);
	///
	void ScreenOptions();
	///
	void showCopyright();
	///
	void showLicense();
	///
	void MenuDocu(string const & docname);
	///
	void handleBufferMenu(int choice);
	
	/// 
	BufferView *currentView();
	///
	LyXView *_view;
	///
	FL_OBJECT *menu_grp1;
	///
	FL_OBJECT *menu_grp2;
	///
	FL_OBJECT *menu_file;
	///
	FL_OBJECT *menu_file2;
	///
	FL_OBJECT *menu_edit;
	///
	FL_OBJECT *menu_layout;
	///
	FL_OBJECT *menu_insert;
	///
	FL_OBJECT *menu_math;
	///
        FL_OBJECT *menu_options;
	///
        FL_OBJECT *menu_options2;
	///
	FL_OBJECT *menu_buffer;
	///
	FL_OBJECT *menu_help;
	///
	FL_OBJECT *menu_help2;
};

#endif
