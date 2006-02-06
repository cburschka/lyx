/**
 * \file GPreferences.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
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

#include "GPreferences.h"
#include "ControlPrefs.h"
#include "ghelpers.h"

//#include "support/lstrings.h"

//#include <boost/tuple/tuple.hpp>

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GPreferences::GPreferences(Dialog & parent)
	: GViewCB<ControlPrefs, GViewGladeB>(parent, _("Preferences"), false)
{}


void GPreferences::doBuild()
{
	string const gladeName = findGladeFile("preferences");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button *button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("OK", button);
	setOK(button);
	xml_->get_widget("Apply", button);
	setApply(button);
	xml_->get_widget("Revert", button);
	setRestore(button);


	// *** Screen fonts ***
	// FIXME: these font buttons display a dialog
	// with options for size and bold/etc which are
	// ignored
	xml_->get_widget("Roman", romanfontbutton_);
	xml_->get_widget("SansSerif", sansseriffontbutton_);
	xml_->get_widget("TypeWriter", typewriterfontbutton_);
	Gtk::SpinButton *spin;
	xml_->get_widget("ScreenDPI", spin);
	dpiadj_ = spin->get_adjustment();
	xml_->get_widget("Zoom", spin);
	zoomadj_ = spin->get_adjustment();

/*
	inlineradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GPreferences::apply));
		bcview().addReadOnly(inlineradio_);*/

}


void GPreferences::update()
{
	applylock_ = true;

	LyXRC const & rc(controller().rc());

	// *** Screen fonts ***
	std::cerr << "Update: got font_name:font_foundry:\n";
	std::cerr << rc.roman_font_name << ":" << rc.roman_font_foundry << "\n";
	std::cerr << rc.sans_font_name << ":" << rc.sans_font_foundry << "\n";
	std::cerr << rc.typewriter_font_name << ":" << rc.typewriter_font_foundry << "\n\n";

	romanfontbutton_->set_font_name(rc.roman_font_name);
	sansseriffontbutton_->set_font_name(rc.sans_font_name);
	typewriterfontbutton_->set_font_name(rc.typewriter_font_name);

	zoomadj_->set_value (rc.zoom);
	dpiadj_->set_value (rc.dpi);

	bc().valid();
	applylock_ = false;
}


void GPreferences::apply()
{
	if (applylock_)
		return;

	LyXRC & rc(controller().rc());

	// *** Screen fonts ***
	rc.roman_font_name = Pango::FontDescription(
		romanfontbutton_->get_font_name()).get_family ();
	rc.roman_font_foundry = "";
	rc.sans_font_name = Pango::FontDescription(
		sansseriffontbutton_->get_font_name()).get_family ();
	rc.sans_font_foundry = "";
	rc.typewriter_font_name = Pango::FontDescription(
		typewriterfontbutton_->get_font_name()).get_family ();
	rc.typewriter_font_foundry = "";

	rc.zoom = zoomadj_->get_value();
	rc.dpi = dpiadj_->get_value();

	// Prevent Apply button ever getting disabled
	bc().valid();
}

} // namespace frontend
} // namespace lyx
