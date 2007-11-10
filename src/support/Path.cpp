/**
 * \file Path.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Needed to prevent the definition of the unnamed_Path macro
// in the header file.

#define PATH_C

#include "support/Path.h"
#include "support/lyxlib.h"


namespace lyx {
namespace support {

PathChanger::PathChanger(FileName const & path)
	: popped_(false)
{
	if (!path.empty()) {
		pushedDir_ = getcwd();

		if (pushedDir_.empty() || chdir(path)) {
			/* FIXME: throw */
		}
	} else {
		popped_ = true;
	}
}


PathChanger::~PathChanger()
{
	if (!popped_)
		pop();
}


int PathChanger::pop()
{
	if (popped_) {
		// should throw an exception
		// throw logical_error();
		return 0;
	}

	if (chdir(pushedDir_)) {
		// should throw an exception
		// throw DirChangeError();
	}
	popped_ = true;

	return 0;
}

} // namespace support
} // namespace lyx
