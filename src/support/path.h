// -*- C++ -*-
#ifndef PATH_H
#define PATH_H

#include <unistd.h>
#include "LString.h"
#include "gettext.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "lyxlib.h"

class Path {
public:
	///
	Path(string const & path)
		: popped_(false)
	{
		if (!path.empty()) { 
			pushedDir_ = GetCWD();
			if (pushedDir_.empty() || lyx::chdir(path.c_str())) {
				WriteFSAlert(_("Error: Could not change to directory: "), 
					     path);
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
	int pop()
	{
		if (popped_) {
			WriteFSAlert(_("Error: Dir already popped: "),
				     pushedDir_);
			return 0;
		}
		if (lyx::chdir(pushedDir_.c_str())) {
			WriteFSAlert(
				_("Error: Could not change to directory: "), 
				pushedDir_);
		}
		popped_ = true;
		return 0;
	}
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
#define Path(x) unnamed_Path;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal

#endif
