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

#include "controllers/frnt_lang.h"
#include "controllers/helper_funcs.h"
#include "support/lstrings.h"

#include <libglademm.h>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

GPreferences::GPreferences(Dialog & parent)
	: GViewCB<ControlPrefs, GViewGladeB>(parent, _("Preferences"), false)
{}


void GPreferences::doBuild()
{
	std::cerr << ">>doBuild\n";
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

	Gtk::HBox *box = NULL;

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
	xml_->get_widget("FirstKeyboardMap", keyboardmap1fcbutton_);
	xml_->get_widget("SecondKeyboardMap", keyboardmap2fcbutton_);

	Gtk::FileFilter kmapfilter;
	kmapfilter.set_name ("LyX keyboard maps");
	kmapfilter.add_pattern ("*.kmap");
	Gtk::FileFilter allfilter;
	allfilter.set_name ("All files");
	allfilter.add_pattern ("*");

	keyboardmap1fcbutton_->add_filter (kmapfilter);
	keyboardmap1fcbutton_->add_filter (allfilter);
	keyboardmap1fcbutton_->set_filter (kmapfilter);
	keyboardmap2fcbutton_->add_filter (kmapfilter);
	keyboardmap2fcbutton_->add_filter (allfilter);
	keyboardmap2fcbutton_->set_filter (kmapfilter);

	keyboardmapcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GPreferences::keyboard_sensitivity));

	// *** Language ***

	xml_->get_widget("DefaultLanguage", box);
	box->pack_start (defaultlanguagecombo_);
	defaultlanguagecombo_.show();
	xml_->get_widget("LanguagePackage", languagepackageentry_);
	xml_->get_widget("CommandStart", commandstartentry_);
	xml_->get_widget("CommandEnd", commandendentry_);
	xml_->get_widget("UseBabel", usebabelcheck_);
	xml_->get_widget("MarkForeignLanguages", markforeigncheck_);
	xml_->get_widget("Global", globalcheck_);
	xml_->get_widget("RTLSupport", RTLsupportcheck_);
	xml_->get_widget("AutoBegin", autobegincheck_);
	xml_->get_widget("AutoEnd", autoendcheck_);

	// Store the lang identifiers for later
	vector<LanguagePair> const langs = getLanguageData(false);
	lang_ = getSecond(langs);

	vector<LanguagePair>::const_iterator lit  = langs.begin();
	vector<LanguagePair>::const_iterator const lend = langs.end();
	for (; lit != lend; ++lit) {
		defaultlanguagecombo_.append_text(lit->first);
	}

	// *** Spellchecker ***
	xml_->get_widget("Spellchecker", box);
	box->pack_start(spellcheckercombo_);
	spellcheckercombo_.show();
	xml_->get_widget("AlternativeLanguage", alternativelanguageentry_);
	xml_->get_widget("EscapeCharacters", escapecharactersentry_);
	xml_->get_widget("PersonalDictionary", personaldictionaryfcbutton_);
	xml_->get_widget("AcceptCompoundWords", acceptcompoundcheck_);
	xml_->get_widget("UseInputEncoding", useinputenccheck_);

	Gtk::FileFilter ispellfilter;
	ispellfilter.set_name ("iSpell Dictionary Files");
	ispellfilter.add_pattern ("*.ispell");

	personaldictionaryfcbutton_->add_filter (ispellfilter);
	personaldictionaryfcbutton_->add_filter (allfilter);
	personaldictionaryfcbutton_->set_filter (ispellfilter);

	spellcheckercombo_.append_text ("ispell");
	spellcheckercombo_.append_text ("aspell");
	spellcheckercombo_.append_text ("hspell");
#ifdef USE_PSPELL
	spellcheckercombo_.append_text(lyx::to_utf8(_("pspell (library)")));
#else
#ifdef USE_ASPELL
	spellcheckercombo_.append_text(lyx::to_utf8(_("aspell (library)")));
#endif
#endif


}


void GPreferences::update()
{
	LyXRC const & rc(controller().rc());

	std::cerr << ">> update\n";

	// *** Screen fonts ***
	Glib::ustring gtk_roman_font_name = rc.roman_font_name + ", 12";
	std::cerr << "Rc's roman_font_name is '" << rc.roman_font_name << "'\n";
	std::cerr << "Our roman_font_name is '" << gtk_roman_font_name << "'\n";
	romanfontbutton_->set_font_name(gtk_roman_font_name);
	Glib::ustring gtk_sans_font_name = rc.sans_font_name + ", 12";
	sansseriffontbutton_->set_font_name(gtk_sans_font_name);
	Glib::ustring gtk_typewriter_font_name = rc.typewriter_font_name + ", 12";
	typewriterfontbutton_->set_font_name(gtk_typewriter_font_name);

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
	keyboardmap1fcbutton_->set_filename (rc.primary_kbmap);
	keyboardmap2fcbutton_->set_filename (rc.secondary_kbmap);
	keyboardmap1fcbutton_->set_sensitive (rc.use_kbmap);
	keyboardmap2fcbutton_->set_sensitive (rc.use_kbmap);

	// *** Language ***
	int const pos = int(findPos(lang_, rc.default_language));
	defaultlanguagecombo_.set_active(pos);

	languagepackageentry_->set_text(rc.language_package);
	commandstartentry_->set_text(rc.language_command_begin);
	commandendentry_->set_text(rc.language_command_end);

	usebabelcheck_->set_active(rc.language_use_babel);
	markforeigncheck_->set_active(rc.mark_foreign_language);
	globalcheck_->set_active(rc.language_global_options);
	RTLsupportcheck_->set_active(rc.rtl_support);
	autobegincheck_->set_active(rc.language_auto_begin);
	autoendcheck_->set_active(rc.language_auto_end);

	// *** Spellchecker ***
	spellcheckercombo_.set_active (0);

	if (rc.isp_command == "ispell") {
		spellcheckercombo_.set_active (0);
	} else if (rc.isp_command == "aspell") {
		spellcheckercombo_.set_active (1);
	} else if (rc.isp_command == "hspell") {
		spellcheckercombo_.set_active (2);
	}

	if (rc.use_spell_lib) {
#if defined(USE_ASPELL) || defined(USE_PSPELL)
		spellcheckercombo_.set_active (3);
#endif
	}

	// FIXME: remove isp_use_alt_lang
	alternativelanguageentry_->set_text(rc.isp_alt_lang);
	// FIXME: remove isp_use_esc_chars
	escapecharactersentry_->set_text(rc.isp_esc_chars);
	// FIXME: remove isp_use_pers_dict
	personaldictionaryfcbutton_->set_filename(rc.isp_pers_dict);
	acceptcompoundcheck_->set_active(rc.isp_accept_compound);
	useinputenccheck_->set_active(rc.isp_use_input_encoding);

	bc().valid();
}


void GPreferences::apply()
{
	LyXRC & rc(controller().rc());

	// *** Screen fonts ***
	LyXRC const oldrc(rc);

	rc.roman_font_name = Pango::FontDescription(
		romanfontbutton_->get_font_name()).get_family ();
	std::cerr << "Button's font_name is '" <<	romanfontbutton_->get_font_name() << "'\n";
	std::cerr << "Setting font name to :\n";
	std::cerr << "\t'" << Pango::FontDescription(
		romanfontbutton_->get_font_name()).get_family () << "'\n";
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
		rc.primary_kbmap = keyboardmap1fcbutton_->get_filename();
		rc.secondary_kbmap = keyboardmap2fcbutton_->get_filename();
	}

	// *** Language ***
	rc.default_language = lang_[
		defaultlanguagecombo_.get_active_row_number()];

	rc.language_package = languagepackageentry_->get_text();
	rc.language_command_begin = commandstartentry_->get_text();
	rc.language_command_end = commandendentry_->get_text();

	rc.language_use_babel = usebabelcheck_->get_active();
	rc.mark_foreign_language = markforeigncheck_->get_active();
	rc.language_global_options = globalcheck_->get_active();
	rc.rtl_support = RTLsupportcheck_->get_active();
	rc.language_auto_begin = autobegincheck_->get_active();
	rc.language_auto_end = autoendcheck_->get_active();

	// *** Spellchecker ***
	switch (spellcheckercombo_.get_active_row_number()) {
		case 0:
		case 1:
		case 2:
			rc.use_spell_lib = false;
			rc.isp_command = spellcheckercombo_.get_active_text();
			break;
		case 3:
			rc.use_spell_lib = true;
			break;
	}

	// FIXME: remove isp_use_alt_lang
	rc.isp_alt_lang = alternativelanguageentry_->get_text();
	rc.isp_use_alt_lang = !rc.isp_alt_lang.empty();
	// FIXME: remove isp_use_esc_chars
	rc.isp_esc_chars = escapecharactersentry_->get_text();
	rc.isp_use_esc_chars = !rc.isp_esc_chars.empty();
	// FIXME: remove isp_use_pers_dict
	rc.isp_pers_dict = personaldictionaryfcbutton_->get_filename();
	rc.isp_use_pers_dict = !rc.isp_pers_dict.empty();
	rc.isp_accept_compound = acceptcompoundcheck_->get_active();
	rc.isp_use_input_encoding = useinputenccheck_->get_active();

	// Prevent Apply button ever getting disabled
	bc().valid();
}


void GPreferences::keyboard_sensitivity ()
{
	bool const kbmap = keyboardmapcheck_->get_active();
	keyboardmap1fcbutton_->set_sensitive(kbmap);
	keyboardmap2fcbutton_->set_sensitive(kbmap);
}


} // namespace frontend
} // namespace lyx
