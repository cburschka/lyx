/**
 * \file ghelpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ghelpers.h"

#include "debug.h"
#include "lengthcommon.h"

#include "support/filetools.h"
#include "support/path_defines.h"

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

vector<string> const buildLengthUnitList()
{
	vector<string> data(unit_name_gui, unit_name_gui + num_units);

	return data;
}


string const findGladeFile(string const & name)
{
	// First, search in the installation directories.

	string filename = lyx::support::LibFileSearch("glade", name, "glade");

	if (!filename.empty())
		return filename;

	// Second, search in the src tree.
	string const dir =
		lyx::support::AddPath(lyx::support::top_srcdir(),
				      "src/frontends/gtk/glade");

	filename = lyx::support::ChangeExtension(name, ".glade");
	filename = lyx::support::AddName(dir, filename);

	if (!lyx::support::IsFileReadable(filename)) {
		lyxerr << "Unable to find glade file \"" << name
		       << "\". libglade is going to crash..." << std::endl;
	}

	return filename;
}

} // namespace frontend
} // namespace lyx
