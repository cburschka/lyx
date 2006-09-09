/**
 * \file GAboutlyx.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
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

#include "ControlAboutlyx.h"
#include "GAboutlyx.h"
#include "ghelpers.h"
#include "version.h"

#include "support/filetools.h" // LibFileSearch

#include <libglademm.h>

#include <sstream>

using lyx::support::libFileSearch;

using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {

namespace {

enum TranslateState {NORMAL, BEGIN, IN_AT, IN_BOLD, IN_ITALIC};


Glib::ustring translateMarkup(Glib::ustring const & lyxMarkup)
{
	Glib::ustring::const_iterator it = lyxMarkup.begin();
	Glib::ustring pangoMarkup;
	TranslateState state = BEGIN;
	for (; it != lyxMarkup.end(); it++) {
		switch (state) {
		case BEGIN:
			switch (*it) {
			case '@':
				state = IN_AT;
				break;
			case '\n':
				state = BEGIN;
				pangoMarkup.push_back('\n');
				break;
			default:
				state = NORMAL;
				pangoMarkup.push_back(*it);
				break;
			}
			break;
		case IN_AT:
			switch (*it) {
			case 'b':
				state = IN_BOLD;
				pangoMarkup += "<b>";
				break;
			case 'i':
				state = IN_ITALIC;
				pangoMarkup += "<i>";
				break;
			case '\n':
				state = BEGIN;
				pangoMarkup.push_back('@');
				pangoMarkup.push_back('\n');
				break;
			default:
				state = NORMAL;
				pangoMarkup.push_back('@');
				pangoMarkup.push_back(*it);
			}
			break;
		case IN_BOLD:
			switch (*it) {
			case '\n':
				state = BEGIN;
				pangoMarkup += "</b>\n";
				break;
			default:
				pangoMarkup.push_back(*it);
			}
			break;
		case IN_ITALIC:
			switch (*it) {
			case '\n':
				state = BEGIN;
				pangoMarkup += "</i>\n";
				break;
			default:
				pangoMarkup.push_back(*it);
			}
			break;
		case NORMAL:
			switch (*it) {
			case '\n':
				state = BEGIN;
				pangoMarkup.push_back('\n');
				break;
			default:
				pangoMarkup.push_back(*it);
			}
		}
		switch (*it) {
		case '&':
			pangoMarkup += "amp;";
			break;
		case '<':
			pangoMarkup.erase(--(pangoMarkup.end()));
			pangoMarkup += "&lt;";
			break;
		case '>':
			pangoMarkup.erase(--(pangoMarkup.end()));
			pangoMarkup += "&gt;";
			break;
		default:
			break;
		}
	}
	switch (state) {
	case IN_AT:
		pangoMarkup.push_back('@');
		break;
	case IN_ITALIC:
		pangoMarkup += "</i>";
		break;
	case IN_BOLD:
		pangoMarkup += "</b>";
		break;
	default:
		break;
	}
	return pangoMarkup;
}


}


GAboutlyx::GAboutlyx(Dialog & parent)
	: GViewCB<ControlAboutlyx, GViewGladeB>(parent, "About LyX")
{
}


void GAboutlyx::doBuild()
{
	string const gladeName = findGladeFile("aboutlyx");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::AboutDialog *dialog;
	xml_->get_widget("dialog", dialog);

	dialog->set_version(Glib::ustring(PACKAGE_VERSION));

	std::ostringstream ls;
	ls << controller().getCopyright() << "\n\n"
	   << controller().getLicense() << "\n\n"
	   << controller().getDisclaimer();
	dialog->set_license (ls.str());

	string const filename = libFileSearch("images", "banner", "ppm");
	Glib::RefPtr<Gdk::Pixbuf> logo = Gdk::Pixbuf::create_from_file(filename);
	Glib::RefPtr<Gdk::Pixbuf> logo_scaled = logo->scale_simple(
		logo->get_width() / 2,
		logo->get_height() / 2,
		Gdk::INTERP_BILINEAR);
	dialog->set_logo(logo_scaled);

	// Total crack - find and hide the built in Credits button
	// that glade helpfully puts there for us.
	Glib::List_Iterator<Gtk::Box_Helpers::Child> it =
		dialog->get_action_area()->children().begin();
	Glib::List_Iterator<Gtk::Box_Helpers::Child> const end =
		dialog->get_action_area()->children().end();
	for (; it != end; ++it) {
		Gtk::Button * button = (Gtk::Button*)(it->get_widget());
		// The close button is a stock ID which we can reliably test for
		// The license button has no icon
		// What's left is the credits button
		if (button->get_label() != "gtk-close" && button->get_image())
			button->hide();
	}

	// FIXME UNICODE
	Gtk::Button &authorbutton = *Gtk::manage(
		new Gtk::Button(lyx::to_utf8(_("C_redits")), true));
	authorbutton.set_image(*Gtk::manage(
		new Gtk::Image(Gtk::Stock::ABOUT, Gtk::ICON_SIZE_BUTTON)));
	dialog->get_action_area()->pack_end(authorbutton);
	dialog->get_action_area()->reorder_child(authorbutton, 0);
	authorbutton.signal_clicked().connect(
		sigc::mem_fun(*this, &GAboutlyx::showAuthors));
	authorbutton.show();
	xml_->get_widget("AuthorsDialog", authordialog_);
	Gtk::Label *authorlabel;
	xml_->get_widget("Authors", authorlabel);
	std::ostringstream crs;
	controller().getCredits(crs);
	authorlabel->set_markup(translateMarkup(
		Glib::convert(crs.str(), "UTF-8", "ISO-8859-1")));
}


void GAboutlyx::showAuthors()
{
	authordialog_->run();
	authordialog_->hide();
}


} // namespace frontend
} // namespace lyx
