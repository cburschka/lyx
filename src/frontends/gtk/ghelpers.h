// -*- C++ -*-
/**
 * \file ghelpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GHELPERS_H
#define GHELPERS_H

#include "lengthcommon.h"

#include <gtkmm.h>

#include <string>
#include <vector>

class FuncRequest;

namespace lyx {
namespace frontend {

// Get a GTK stockID from a lyx function id.
// Return Gtk::Stock::MISSING_IMAGE if no suitable stock found
Gtk::BuiltinStockID getGTKStockIcon(FuncRequest const & func);

std::string const getDefaultUnit();

void unitsComboFromLength(Gtk::ComboBox * combo,
                           Gtk::TreeModelColumn<Glib::ustring> const & stringcol,
                           LyXLength const & len,
                           std::string defunit);

std::vector<std::string> const buildLengthUnitList();

std::vector<std::string> const buildLengthNoRelUnitList();

/** name is the name of the glade file, without path or extension.
 *  Eg, "aboutlyx", "tableCreate".
 */
std::string const findGladeFile(std::string const & name);

template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return std::distance(vec.begin(), it);
}

} // namespace frontend
} // namespace lyx

#endif // NOT GHELPERS_H
