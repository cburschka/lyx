// -*- C++ -*-
/**
 * \file XFormsToolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFORMSTOOLBAR_H
#define XFROMSTOOLBAR_H

#include <vector>
#include "forms_fwd.h"

#include "frontends/Toolbar.h"
#include "ToolbarBackend.h"

class XFormsView;
class Tooltips;

/** The LyX xforms toolbar class
 */
class XFormsToolbar : public Toolbar {
public:
	/// create an empty toolbar
	XFormsToolbar(LyXView * o, int x, int y);

	///
	~XFormsToolbar();

	/// add a new toolbar
	void add(ToolbarBackend::Toolbar const & tb);

	/// add an item to a toolbar
	void add(int action, string const & tooltip);

	/// display toolbar, not implemented
	void displayToolbar(ToolbarBackend::Toolbar const & tb, bool show);

	/// update the state of the icons
	void update();

	/// select the right layout in the combox
	void setLayout(string const & layout);
	/// Populate the layout combox.
	void updateLayoutList();
	/// Drop down the layout list
	void openLayoutList();
	/// Erase the layout list
	void clearLayoutList();
	///
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
	FL_OBJECT * combox_;
	/// x position of end of toolbar
	int xpos;
	/// y position of end of toolbar
	int ypos;
};

#endif
