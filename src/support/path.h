// -*- C++ -*-
/**
 * \file path.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#ifndef PATH_H
#define PATH_H

#include "LString.h"
//#include "filetools.h"
#include "lyxlib.h"
#include <boost/utility.hpp>

#ifdef __GNUG__
#pragma interface
#endif


/**
 * Path - utility closs for stackable working directories
 *
 * You can use a local variable of this type to temporarily
 * change to a directory as the cwd, for example :
 *
 * if (blah) {
 *	Path p("/tmp/blah");
 * 	...
 * }
 *
 * At the end of p's scope the cwd is reset to its previous value.
 */
class Path : boost::noncopyable {
public:
	/// change to the given directory
	explicit
	Path(string const & path)
		: popped_(false)
	{
		if (!path.empty()) { 
			pushedDir_ = lyx::getcwd();
			if (pushedDir_.empty() || lyx::chdir(path))
				/* FIXME: throw */;
		} else {
			popped_ = true;
		}
	}
 
	/// set cwd to the previous value if needed
	~Path()
	{
		if (!popped_) pop();
	}
 
	/// set cwd to the previous value if needed
	int pop();
private:
	/// whether we are in the new cwd or not
	bool popped_;
	/// the previous cwd
	string pushedDir_;
};

// To avoid the wrong usage:
// Path("/tmp");   // wrong
// Path p("/tmp");  // right
// we add this macro:
///
#define Path(x) unnamed_Path;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal

#endif // PATH_H
