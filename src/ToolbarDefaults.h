// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *
 * ====================================================== */


#ifndef TOOLBARDEFAULTS_H
#define TOOLBARDEFAULTS_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>

#include "LString.h"

class LyXLex;

///
class ToolbarDefaults {
public:
        /// The special toolbar actions
	enum  ItemType {
		/// adds space between buttons in the toolbar
		SEPARATOR=-3,
		/// a special combox insead of a button
		LAYOUTS=-2,
		/// begin a new line of button (not working)
		NEWLINE=-1
	};

	///
	typedef std::vector<int> Defaults;
	///
	typedef Defaults::iterator iterator;
	///
	typedef Defaults::const_iterator const_iterator;
	///
	ToolbarDefaults();
	///
	iterator begin() {
		return defaults.begin();
	}
	///
	const_iterator begin() const {
		return defaults.begin();
	}
	///
	iterator end() {
		return defaults.end();
	}
	///
	const_iterator end() const {
		return defaults.end();
	}
	///
	void read(LyXLex &);
private:
	///
	void init();
	/// This func is just to make it easy for me...
	void add(int);
	///
	void add(string const &);
	///
	Defaults defaults;
};

/// The global instance
extern ToolbarDefaults toolbardefaults;


#endif

