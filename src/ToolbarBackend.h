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

	///
	typedef std::vector<int> Items;
	///
	typedef Items::iterator iterator;
	///
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
private:
	/// This func is just to make it easy for me...
	void add(int);
	///
	void add(string const &);
	///
	Items items;
};

/// The global instance
extern ToolbarBackend toolbarbackend;


#endif // TOOLBAR_BACKEND_H
