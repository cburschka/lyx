/**
 * \file ghelpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "ghelpers.h"

#include "lyxrc.h"
#include "funcrequest.h"
#include "debug.h"

#include "support/filetools.h"
#include "support/package.h"

#include <sstream>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

string const getLengthFromWidgets(Gtk::Adjustment const & adj, Gtk::ComboBoxText const & combo)
{
	std::ostringstream os;
	os << adj.get_value();
	os << combo.get_active_text();
	return os.str();
}


void setWidgetsFromLength(Gtk::Adjustment & adj, Gtk::ComboBoxText & combo, LyXLength const & length)
{
	adj.set_value(length.value());

	string unit = stringFromUnit(length.unit());
	if (unit.empty())
		unit = getDefaultUnit();

	comboBoxTextSet(combo,unit);
}


void populateUnitCombo(Gtk::ComboBoxText & combo, bool const userelative)
{
	vector<string> units = buildLengthUnitList(userelative);

	vector<string>::const_iterator it = units.begin();
	vector<string>::const_iterator end = units.end();
	for(; it != end; ++it)
		combo.append_text(*it);
}


int comboBoxTextSet(Gtk::ComboBoxText & combo, Glib::ustring target)
{
	int const children = combo.get_model()->children().size();
	for (int i = 0; i < children; i++) {
		combo.set_active(i);
		if (combo.get_active_text() == target)
			return 0;
	}
	return -1;
}


Gtk::BuiltinStockID getGTKStockIcon(FuncRequest const & func)
{
	switch (func.action) {
		case LFUN_MENUWRITE: return Gtk::Stock::SAVE;
		case LFUN_MENUNEW: return Gtk::Stock::NEW;
		case LFUN_WRITEAS: return Gtk::Stock::SAVE_AS;
		case LFUN_CENTER: return Gtk::Stock::JUSTIFY_CENTER;
		case LFUN_TOCVIEW: return Gtk::Stock::INDEX;
		case LFUN_CLOSEBUFFER: return Gtk::Stock::CLOSE;
		case LFUN_QUIT: return Gtk::Stock::QUIT;
		case LFUN_UNDO: return Gtk::Stock::UNDO;
		case LFUN_REDO: return Gtk::Stock::REDO;
		case LFUN_PASTE: return Gtk::Stock::PASTE;
		case LFUN_PASTESELECTION: return Gtk::Stock::PASTE;
		case LFUN_CUT: return Gtk::Stock::CUT;
		case LFUN_COPY: return Gtk::Stock::COPY;
		case LFUN_BOLD: return Gtk::Stock::BOLD;
		case LFUN_ITAL: return Gtk::Stock::ITALIC;
		case LFUN_FILE_OPEN: return Gtk::Stock::OPEN;
		case LFUN_RECONFIGURE: return Gtk::Stock::REFRESH;
		case LFUN_DIALOG_SHOW:
			if (func.argument == "findreplace")
				return Gtk::Stock::FIND_AND_REPLACE;
			else if (func.argument == "print")
				return Gtk::Stock::PRINT;
			else if (func.argument == "spellchecker")
				return Gtk::Stock::SPELL_CHECK;
			else if (func.argument == "prefs")
				return Gtk::Stock::PREFERENCES;
			else if (func.argument == "document")
				return Gtk::Stock::PROPERTIES;
			else
				return Gtk::Stock::MISSING_IMAGE;
			break;
		default: return Gtk::Stock::MISSING_IMAGE;
	}
}


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
	// shut up compiler
	return "cm";
}


void unitsComboFromLength(Gtk::ComboBox * combo,
                           Gtk::TreeModelColumn<Glib::ustring> const & stringcol,
                           LyXLength const & len,
                           std::string const & defunit)
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


vector<string> const buildLengthUnitList(bool const userelative)
{
	//vector<string> data(unit_name_gui, unit_name_gui + num_units);
	vector<string> data;
	if (userelative) {
		data = vector<string>(unit_name_gui, unit_name_gui + num_units);
	} else {
		for (int i = 0; i < num_units; ++i) {
			string str(unit_name_gui[i]);
			if (str.find("%") == string::npos)
				data.push_back(unit_name_gui[i]);
		}
	}
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
		lyx::support::AddPath(lyx::support::package().top_srcdir(),
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
