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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class LyXView;
class ToolbarDefaults;

/** The LyX GUI independent toolbar class
  The GUI interface is implemented in the corresponding Toolbar_pimpl class.
  */
class Toolbar {
public:
	///
	Toolbar(LyXView * o, int x, int y, ToolbarDefaults const &);

	///
	~Toolbar();

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
	/// update the state of the icons
	void update();

	/// update the layout combox
	void setLayout(string const & layout);
	/// Populate the layout combox; re-do everything if force is true.
	void updateLayoutList(bool force);
	/// Drop down the layout list
	void openLayoutList();
	/// Erase the layout list
	void clearLayoutList();
	/** This is a fudge needed to overcome some wierd xforms behaviour.
	    If the tooltips are set before the dialog is visible, then they
	    are not displayed. No, I don't understand either.

	    This is a fix for LyX branch 1_2_X. It will not be ported forward
	    into 1.3 and beyond; a better fix should be found.
	 */
	void setTooltips();

	/// Compaq cxx 6.5 requires this to be public
	struct Pimpl;
private:
	///
	friend struct Toolbar::Pimpl;
	///
	Pimpl * pimpl_;
};
#endif
