// -*- C++ -*-
/**
 * \file Toolbar_pimpl.h
 * Copyright 2002 the LyX Team
 * Copyright 1996-2001 Lars Gullik Bjønnes
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 */

#ifndef TOOLBAR_PIMPL_H
#define TOOLBAR_PIMPL_H

#include <vector>
#include "forms_fwd.h"

#include "frontends/Toolbar.h"

#ifdef __GNUG__
#pragma interface
#endif

class XFormsView;
class Tooltips;
class Dialogs;
class Combox;

/** The LyX xforms toolbar class
  */
struct Toolbar::Pimpl {
public:
	/// called when user selects a layout from combox
	static void layoutSelectedCB(int, void *, Combox *);
 
	/// create an empty toolbar
	Pimpl(LyXView * o, Dialogs &, int x, int y);

	~Pimpl();

	/// add a new button to the toolbar.
	void add(int action);

	/// update the state of the icons
	void update();

	/// select the right layout in the combox
	void setLayout(string const & layout);
	/// Populate the layout combox; re-do everything if force is true.
	void updateLayoutList(bool force);
	/// Drop down the layout list
	void openLayoutList();
	/// Erase the layout list
	void clearLayoutList();
	/// the non-static version of layoutSelectedCB
	void layoutSelected();

	/// an item on the toolbar
	struct toolbarItem
	{
		toolbarItem();

		~toolbarItem();
 
		toolbarItem & operator=(toolbarItem const & ti);
 
		/// deallocate icon
		void kill_icon();
 
		/// lyx action number
		int action;
		/// icon for this item
		FL_OBJECT * icon;
	};

	typedef std::vector<toolbarItem> ToolbarList;
 
	/// The list containing all the buttons
	ToolbarList toollist_;
	/// owning view
	XFormsView * owner_;
	/// tooltips manager
	Tooltips * tooltip_;
	/// layout combo
	Combox * combox_;
	/// x position of end of toolbar
	int xpos;
	/// y position of end of toolbar
	int ypos;
};
 
#endif // TOOLBAR_PIMPL_H
