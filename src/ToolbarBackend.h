// -*- C++ -*-
/**
 * \file ToolbarBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef TOOLBAR_BACKEND_H
#define TOOLBAR_BACKEND_H

#include <vector>
#include <algorithm>

#include "LString.h"

class LyXLex;

///
class ToolbarBackend {
public:
	/// The special toolbar actions
	enum ItemType {
		/// adds space between buttons in the toolbar
		SEPARATOR = -3,
		/// a special combox insead of a button
		LAYOUTS = -2,
		/// begin a new line of button (not working)
		NEWLINE = -1
	};

	/// action, tooltip
	typedef std::pair<int, string> Item;

	/// the toolbar items
	typedef std::vector<std::pair<int, string> > Items;

	/// possibly display types
	enum DisplayType {
		OFF, //< never shown
		ON, //< always shown
		MATH, //< shown when in math
		TABLE //< shown when in table
	};

	/// a toolbar
	struct Toolbar {
		/// toolbar UI name
		string name;
		/// toolbar contents
		Items items;
		/// display type
		DisplayType display_type;
	};

	typedef std::vector<Toolbar> Toolbars;

	typedef Items::const_iterator item_iterator;

	ToolbarBackend();

	/// iterator for all toolbars
	Toolbars::const_iterator begin() const {
		return toolbars.begin();
	}

	Toolbars::const_iterator end() const {
		return toolbars.end();
	}

	/// read a toolbar from the file
	void read(LyXLex &);

	/// return a full path of an XPM for the given action
	static string const getIcon(int action);

private:
	/// add the given lfun with tooltip if relevant
	void add(Toolbar & tb, int, string const & tooltip = string());

	/// add the given lfun with tooltip if relevant
	void add(Toolbar & tb, string const &, string const & tooltip);

	/// all the toolbars
	Toolbars toolbars;
};

/// The global instance
extern ToolbarBackend toolbarbackend;


#endif // TOOLBAR_BACKEND_H
