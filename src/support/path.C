/**
 * \file path.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Needed to prevent the definition of the unnamed_Path macro in the header file.
#define PATH_C

#include "path.h"
#include "lyxlib.h"


using std::string;


namespace lyx {
namespace support {

Path::Path(string const & path)
	: popped_(false)
{
	if (!path.empty()) {
		pushedDir_ = getcwd();
		if (pushedDir_.empty() || chdir(path))
			/* FIXME: throw */;
	} else {
		popped_ = true;
	}
}


Path::~Path()
{
	if (!popped_) pop();
}


int Path::pop()
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
