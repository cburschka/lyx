/**
 * \file path.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
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
