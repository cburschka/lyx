
/**
 * \file GtkLengthEntry.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ghelpers.h"

#include "GtkLengthEntry.h"

#include <vector>
#include <sstream>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif


using std::string;
using std::vector;

namespace lyx {
namespace frontend {

namespace {

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
	vector<string> const units = buildLengthUnitList(userelative);

	vector<string>::const_iterator it = units.begin();
	vector<string>::const_iterator const end = units.end();
	for(; it != end; ++it)
		combo.append_text(*it);
}


}


GtkLengthEntry::GtkLengthEntry(
	BaseObjectType* cobject,
	const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::HBox(cobject), adj_(666.0, 0.0, 99999.0, 0.1, 1, 0.0), spin_(adj_, 0.1, 2)
{
	populateUnitCombo (combo_, true);
	relative_ = true;

	set_spacing(6);
	pack_start (spin_, true, true, 0);
	pack_start (combo_, false, false, 0);
	show_all();
	spin_.signal_changed().connect(sigc::mem_fun(changedsignal_, &sigc::signal<void>::emit));
	combo_.signal_changed().connect(sigc::mem_fun(changedsignal_, &sigc::signal<void>::emit));
}


sigc::signal< void >& GtkLengthEntry::signal_changed()
{
	return changedsignal_;
}


void GtkLengthEntry::set_length(LyXLength const & length)
{
	setWidgetsFromLength (*spin_.get_adjustment(), combo_, length);
}


void GtkLengthEntry::set_length(std::string const & length)
{
	setWidgetsFromLength (*spin_.get_adjustment(), combo_, LyXLength(length));
}


LyXLength const GtkLengthEntry::get_length()
{
	return LyXLength(getLengthFromWidgets(*spin_.get_adjustment(), combo_));
}


std::string const GtkLengthEntry::get_length_string()
{
	return getLengthFromWidgets(*spin_.get_adjustment(), combo_);
}


void GtkLengthEntry::set_relative(bool rel)
{
	combo_.clear();
	if (rel != relative_) {
		populateUnitCombo (combo_, rel);
		relative_ = rel;
	}
}


} // namespace frontend
} // namespace lyx
