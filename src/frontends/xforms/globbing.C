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

#include "globbing.h"
#include "support/path.h"

#include <glob.h>

using std::string;
using std::vector;


namespace lyx {
namespace support {

void glob(vector<string> & matches,
	  string const & pattern,
	  string const & working_dir,
	  int flags)
{
	Path p(working_dir);

	glob_t glob_buffer;
	glob_buffer.gl_offs = 0;
	glob(pattern.c_str(), flags, 0, &glob_buffer);

	matches.insert(matches.end(),
		       glob_buffer.gl_pathv,
		       glob_buffer.gl_pathv + glob_buffer.gl_pathc);

	globfree(&glob_buffer);
}

} // namespace support
} // namespace lyx
