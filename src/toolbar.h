// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1996-2000
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

#ifndef TOOLBAR_H
#define TOOLBAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include "lyxfunc.h"
#include "combox.h"

/** The LyX toolbar class
  This class {\em is} the LyX toolbar, and is not likely to be enhanced
  further until we begin the move to Qt. We will probably have to make our
  own QToolBar, at least until Troll Tech makes theirs.
  */
class Toolbar {
public:
	///
	Toolbar(LyXView * o, int x, int y);

	///
	~Toolbar() {
		clean();
	}
	
	///
	int get_toolbar_func(string const & func);
	
        /// The special toolbar actions
	enum  TOOLBARITEMS {
		/// adds space between buttons in the toolbar
		TOOL_SEPARATOR = -1,
		/// a special combox insead of a button
		TOOL_LAYOUTS = -2,
		/// begin a new line of button (not working)
		TOOL_NEWLINE = -3
	};

	///
	Combox * combox;

	/// (re)sets the toolbar
	void set(bool doingmain = false);

	/** this is to be the entry point to the toolbar
	  frame, where you can change the toolbar realtime. */
	void edit();
	/// add a new button to the toolbar.
    	void add(int , bool doclean = true);
	/// name of func instead of kb_action
	void add(string const & , bool doclean = true);
	/// invokes the n'th icon in the toolbar
	void push(int);
	/// activates the toolbar
        void activate();
	/// deactivates the toolbar
        void deactivate();

	///
	static void ToolbarCB(FL_OBJECT *, long);

#if FL_REVISION < 89
	///
	static void BubbleTimerCB(FL_OBJECT *, long);
	///
	static int BubblePost(FL_OBJECT * ob, int event,
			      FL_Coord mx, FL_Coord my, int key, void * xev);
#endif

private:
	///
	struct toolbarItem
	{
		///
		toolbarItem * next;
		///
		int action;
		///
		string help;
		///
		FL_OBJECT * icon;
		///
		bool IsBitmap;
		///
		char const ** pixmap;
		///
		toolbarItem() {
			next = 0;
			action = LFUN_NOACTION;
			icon = 0;
			pixmap = 0;
			IsBitmap = false;
		}
		///
		~toolbarItem(){
			if (icon){
				fl_delete_object(icon);
				fl_free_object(icon);
			}
		}
			
	};

	/// a list containing all the buttons
	toolbarItem * toollist;
	///
	LyXView * owner;
#if FL_REVISION < 89
	///
	FL_OBJECT * bubble_timer;
#endif
	/// Starting position
	int sxpos, sypos;
	///
	int xpos;
	///
	int ypos;
	///
	int buttonwidth;
	///
	int height;
	///
	int standardspacing;
	///
	int sepspace;
	///
	bool cleaned;

	///
	char const ** getPixmap(kb_action, string const & arg = string());
	/// removes all toolbar buttons from the toolbar.
	void clean();

	/** more...
	 */
	void reset(){
		toollist = 0;
		cleaned = false;
		
		lightReset();
	}

	/** more...
	 */
	void lightReset(){
		standardspacing = 2; // the usual space between items
		sepspace = 6; // extra space
		xpos = sxpos - standardspacing;
		ypos = sypos;
		buttonwidth = 30; // the standard button width
		height = 30; // the height of all items in the toolbar
	}
};
#endif
