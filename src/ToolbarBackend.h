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
	enum  ItemType {
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

	typedef Items::iterator iterator;

	typedef Items::const_iterator const_iterator;
	///
	ToolbarBackend();
	///
	iterator begin() {
		return items.begin();
	}
	///
	const_iterator begin() const {
		return items.begin();
	}
	///
	iterator end() {
		return items.end();
	}
	///
	const_iterator end() const {
		return items.end();
	}
	///
	void read(LyXLex &);

	/// return a full path of an XPM for the given action
	static string const getIcon(int action);

private:
	/// add the given lfun with tooltip if relevant
	void add(int, string const & tooltip = string());
	/// add the given lfun with tooltip if relevant
	void add(string const &, string const & tooltip);
	/// all the items
	Items items;
};

/// The global instance
extern ToolbarBackend toolbarbackend;


#endif // TOOLBAR_BACKEND_H
