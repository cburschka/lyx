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

#include "lyxrc.h"
#include "debug.h"

#include "support/filetools.h"
#include "support/path_defines.h"

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

string const getDefaultUnit()
{
	switch (lyxrc.default_papersize) {
		case PAPER_DEFAULT: return "cm";
		case PAPER_USLETTER:
		case PAPER_LEGALPAPER:
		case PAPER_EXECUTIVEPAPER: return "in"; break;
		case PAPER_A3PAPER:
		case PAPER_A4PAPER:
		case PAPER_A5PAPER:
		case PAPER_B5PAPER: return "cm"; break;
	}
}


void unitsComboFromLength(Gtk::ComboBox * combo,
                           Gtk::TreeModelColumn<Glib::ustring> const & stringcol,
                           LyXLength const & len,
                           std::string defunit)
{
	string unit = stringFromUnit(len.unit());
	if (unit.empty())
		unit = defunit;

	Gtk::TreeModel::iterator it = combo->get_model()->children().begin();
	Gtk::TreeModel::iterator end = combo->get_model()->children().end();
	for (; it != end ; ++it) {
		if ((*it)[stringcol] == unit) {
			combo->set_active(it);
			return;
		}
	}

	// Fallen through, we didn't find the target length!
	combo->set_active(0);
	lyxerr << "unitsComboFromLength: couldn't find "
		"target unit '" << unit << "'\n";
}


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
