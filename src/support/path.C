/**
 * \file path.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "path.h"

int Path::pop()
{
	if (popped_) {
		// should throw an exception
		// throw logical_error();
		return 0;
	}
 
	if (lyx::chdir(pushedDir_)) {
		// should throw an exception
		// throw DirChangeError();
	}
	popped_ = true;
 
	return 0;
}
