// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

#ifndef TOOLBAR_PIMPL_H
#define TOOLBAR_PIMPL_H

#include <vector>
#include FORMS_H_LOCATION

#include "frontends/Toolbar.h"

#include "commandtags.h"
#include "combox.h"
#include "ToolbarDefaults.h"

#ifdef __GNUG__
#pragma interface
#endif

class XFormsView;

/** The LyX xforms toolbar class
  */
struct Toolbar::Pimpl {
public:
	/// called when user selects a layout from combox
	static void layoutSelectedCB(int, void *, Combox *);
	///
	Pimpl(LyXView * o, int x, int y);

	///
	~Pimpl() {
		clean();
	}
	
	/// (re)sets the toolbar
	void set(bool doingmain = false);

	/** this is to be the entry point to the toolbar
	    frame, where you can change the toolbar realtime.
	*/
	void edit();
	/// add a new button to the toolbar.
    	void add(int, bool doclean = true);
	/// invokes the n'th icon in the toolbar
	void push(int);
	/// activates the toolbar
        void activate();
	/// deactivates the toolbar
        void deactivate();
	/// update the state of the icons
	void update();

 
	/// select the right layout in the combox
	void setLayout(int layout);
	/// Populate the layout combox; re-do everything if force is true.
	void updateLayoutList(bool force);
	/// Drop down the layout list
	void openLayoutList();
	/// Erase the layout list
	void clearLayoutList();
	/// and the non-static version
	void layoutSelected(int); 

	///
	struct toolbarItem
	{
		///
		int action;
		///
		FL_OBJECT * icon;
		///
		toolbarItem();
		///
		void clean();
		///
		~toolbarItem();
		///
		toolbarItem & operator=(toolbarItem const & ti);
	};

	/// typedef to simplify things
	typedef std::vector<toolbarItem> ToolbarList;
	/// The list containing all the buttons
	ToolbarList toollist;
	///
	XFormsView * owner;
#if FL_REVISION < 89
	///
	FL_OBJECT * bubble_timer;
#endif
	///
	Combox * combox;
	/// Starting position
	int sxpos;
	///
	int sypos;
	///
	int xpos;
	///
	int ypos;
	///
	bool cleaned;

	/// removes all toolbar buttons from the toolbar.
	void clean();

	/// more...
	void reset();

	/// more...
	void lightReset();
};
#endif
