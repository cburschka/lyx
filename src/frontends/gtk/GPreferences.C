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
	
	// *** Graphics ***
	xml_->get_widget("GraphicsColor", graphicscolorradio_);
	xml_->get_widget("GraphicsGrayscale", graphicsgrayscaleradio_);
	xml_->get_widget("GraphicsMonochrome", graphicsmonoradio_);
	xml_->get_widget("GraphicsDoNotDisplay", graphicsnoneradio_);
	
	xml_->get_widget("InstantPreviewOn", instprevonradio_);
	xml_->get_widget("InstantPreviewOff", instprevoffradio_);
	xml_->get_widget("InstantPreviewNoMath", instprevnomathradio_);

	// *** Keyboard ***
	xml_->get_widget("UseKeyboardMap", keyboardmapcheck_);
	Gtk::HBox *box;
	xml_->get_widget("FirstKeyboardMap", box);
	box->pack_start(keyboardmap1fcbutton_);
	keyboardmap1fcbutton_.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
	keyboardmap1fcbutton_.show();
	xml_->get_widget("SecondKeyboardMap", box);
	box->pack_start(keyboardmap2fcbutton_);
	keyboardmap2fcbutton_.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
	keyboardmap2fcbutton_.show();
	
	Gtk::FileFilter kmapfilter;
	kmapfilter.set_name ("LyX keyboard maps");
	kmapfilter.add_pattern ("*.kmap");
	Gtk::FileFilter allfilter;
	allfilter.set_name ("All files");
	allfilter.add_pattern ("*");

	keyboardmap1fcbutton_.add_filter (kmapfilter);
	keyboardmap1fcbutton_.add_filter (allfilter);
	keyboardmap1fcbutton_.set_filter (kmapfilter);
	keyboardmap2fcbutton_.add_filter (kmapfilter);
	keyboardmap2fcbutton_.add_filter (allfilter);
	keyboardmap2fcbutton_.set_filter (kmapfilter);
	
	keyboardmapcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GPreferences::keyboard_sensitivity));
}


void GPreferences::update()
{
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

	// *** Graphics ***
	switch (rc.display_graphics) {
		case graphics::NoDisplay:
			graphicsnoneradio_->set_active();
		break;
		case graphics::MonochromeDisplay:
			graphicsmonoradio_->set_active();
		break;
		case graphics::GrayscaleDisplay:
			graphicsgrayscaleradio_->set_active();
		break;
		default:
		case graphics::ColorDisplay:
			graphicscolorradio_->set_active();
		break;
	}

	switch (rc.preview) {
		case LyXRC::PREVIEW_ON:
			instprevonradio_->set_active();
		break;
		case LyXRC::PREVIEW_NO_MATH:
			instprevnomathradio_->set_active();
		break;
		default:
		case LyXRC::PREVIEW_OFF:
			instprevoffradio_->set_active();
		break;
	}
	
	// *** Keyboard ***
	keyboardmapcheck_->set_active (rc.use_kbmap);
	keyboardmap1fcbutton_.set_filename (rc.primary_kbmap);
	keyboardmap2fcbutton_.set_filename (rc.secondary_kbmap);
	keyboardmap1fcbutton_.set_sensitive (rc.use_kbmap);
	keyboardmap2fcbutton_.set_sensitive (rc.use_kbmap);

	bc().valid();
}


void GPreferences::apply()
{
	LyXRC & rc(controller().rc());

	// *** Screen fonts ***
	LyXRC const oldrc(rc);

	rc.roman_font_name = Pango::FontDescription(
		romanfontbutton_->get_font_name()).get_family ();
	rc.roman_font_foundry = "";
	rc.sans_font_name = Pango::FontDescription(
		sansseriffontbutton_->get_font_name()).get_family ();
	rc.sans_font_foundry = "";
	rc.typewriter_font_name = Pango::FontDescription(
		typewriterfontbutton_->get_font_name()).get_family ();
	rc.typewriter_font_foundry = "";

	rc.zoom = static_cast<int>(zoomadj_->get_value());
	rc.dpi = static_cast<int>(dpiadj_->get_value());

	if (rc.font_sizes != oldrc.font_sizes
		|| rc.roman_font_name != oldrc.roman_font_name
		|| rc.sans_font_name != oldrc.sans_font_name
		|| rc.typewriter_font_name != oldrc.typewriter_font_name
		|| rc.zoom != oldrc.zoom || rc.dpi != oldrc.dpi) {
		controller().updateScreenFonts();
	}

	// *** Graphics ***
	if (graphicsnoneradio_->get_active())
		rc.display_graphics = graphics::NoDisplay;
	else if (graphicsgrayscaleradio_->get_active())
		rc.display_graphics = graphics::GrayscaleDisplay;
	else if (graphicsmonoradio_->get_active())
		rc.display_graphics = graphics::MonochromeDisplay;
	else
		rc.display_graphics = graphics::ColorDisplay;

	if (instprevonradio_->get_active())
		rc.preview = LyXRC::PREVIEW_ON;
	else if (instprevnomathradio_->get_active())
		rc.preview = LyXRC::PREVIEW_NO_MATH;
	else
		rc.preview = LyXRC::PREVIEW_OFF;

	// *** Keyboard ***
	rc.use_kbmap = keyboardmapcheck_->get_active();
	if (rc.use_kbmap) {
		rc.primary_kbmap = keyboardmap1fcbutton_.get_filename();
		rc.secondary_kbmap = keyboardmap2fcbutton_.get_filename();
	}

	// Prevent Apply button ever getting disabled
	bc().valid();
}


void GPreferences::keyboard_sensitivity ()
{
	bool const kbmap = keyboardmapcheck_->get_active();
	keyboardmap1fcbutton_.set_sensitive(kbmap);
	keyboardmap2fcbutton_.set_sensitive(kbmap);
}

} // namespace frontend
} // namespace lyx
