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

#include "gtkmm/icontheme.h"

#include <sstream>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

// Get a GTK stockID from a lyx function id.
// Return Gtk::Stock::MISSING_IMAGE if no suitable stock found
Gtk::BuiltinStockID getGTKStockIcon(FuncRequest const & func)
{
	switch (func.action) {
		case LFUN_MENUWRITE: return Gtk::Stock::SAVE;
		case LFUN_MENUNEW: return Gtk::Stock::NEW;
		case LFUN_WRITEAS: return Gtk::Stock::SAVE_AS;
		case LFUN_MENURELOAD: return Gtk::Stock::REVERT_TO_SAVED;
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


Gtk::Image *getGTKIcon(FuncRequest const & func, Gtk::IconSize const & size)
{
		/*static Glib::RefPtr<Gtk::IconTheme> theme;
		if (!theme)
			theme = Gtk::IconTheme::get_default();*/

		Gtk::Image *image = NULL;

		Gtk::BuiltinStockID const stockID = getGTKStockIcon(func);
		if (stockID != Gtk::Stock::MISSING_IMAGE) {
			// Prefer stock gtk graphics
			image = Gtk::manage(new Gtk::Image(stockID, size));
		} else {
			/*if (func.action == LFUN_MENUNEW) {
				std::cerr << "Looking for icon\n";
				std::vector<Glib::ustring> icons = theme->list_icons("mimetypes");
				for (std::vector<Glib::ustring>::iterator it = icons.begin(); it != icons.end(); ++it) {
					std::cerr << (*it) << "\n";
				}
				std::vector<Glib::ustring> paths = theme->get_search_path();
				for (std::vector<Glib::ustring>::iterator it = paths.begin(); it != paths.end(); ++it) {
					std::cerr << (*it) << "\n";
				}
				std::cerr << theme->get_example_icon_name() << "\n";
				if (theme->has_icon("table-center")) {
					std::cerr << "Found icon\n";
					Glib::RefPtr<Gdk::Pixbuf> pbuf = theme->load_icon("table-center", 64, Gtk::ICON_LOOKUP_USE_BUILTIN);
					image = Gtk::manage(new Gtk::Image(pbuf));
				}
			}*/
		}
		
		return image;
}


string const getDefaultUnit()
{
	switch (lyxrc.default_papersize) {
		case PAPER_DEFAULT: return "cm";
		case PAPER_USLETTER:
		case PAPER_USLEGAL:
		case PAPER_USEXECUTIVE: return "in"; break;
		case PAPER_A3:
		case PAPER_A4:
		case PAPER_A5:
		case PAPER_B5: return "cm"; break;
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
