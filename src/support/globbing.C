/**
 * \file globbing.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/globbing.h"

#include <glob.h>

using std::string;
using std::vector;


namespace lyx {
namespace support {

vector<string> const glob(string const & pattern, int flags)
{
	glob_t glob_buffer;
	glob_buffer.gl_offs = 0;
	glob(pattern.c_str(), flags, 0, &glob_buffer);
	vector<string> const matches(glob_buffer.gl_pathv,
				     glob_buffer.gl_pathv +
				     glob_buffer.gl_pathc);
	globfree(&glob_buffer);
	return matches;
}

} // namespace support
} // namespace lyx
