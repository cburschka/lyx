// -*- C++ -*-
#ifndef PATH_H
#define PATH_H

#include "LString.h"
#include "filetools.h"
#include "lyxlib.h"
#include "utility.hpp"

#ifdef __GNUG__
#pragma interface
#endif

///
class Path : public noncopyable {
public:
	///
	explicit
	Path(string const & path)
		: popped_(false)
	{
		if (!path.empty()) { 
			pushedDir_ = GetCWD();
			if (pushedDir_.empty() || lyx::chdir(path.c_str())) {
				// should throw an exception
				// throw DirChangeError();
				// The use of WriteFSAlert makes this
				// impossible to inline.
				//WriteFSAlert(_("Error: Could not change to directory: "), 
				//	     path);
			}
		} else {
			popped_ = true;
		}
	}
	///
	~Path()
	{
		if (!popped_) pop();
	}
	///
	int pop();
private:
	///
	bool popped_;
	///
	string pushedDir_;
};

// To avoid the wrong usage:
// Path("/tmp");   // wrong
// Path p("/tmp");  // right
// we add this macro:
///
#define Path(x) unnamed_Path;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal

#endif
