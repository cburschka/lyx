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
#include <gtkmm.h>

#include <libglademm.h>
#include <sstream>

#include "ControlAboutlyx.h"
#include "GAboutlyx.h"
#include "support/filetools.h"
#include "version.h"

using std::ostringstream;
using std::string;


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
	string const gladeName =
		lyx::support::LibFileSearch("glade", "aboutlyx", "glade");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Label * version;
	Gtk::Label * credits;
	Gtk::Label * license;
	xml_->get_widget("version", version);
	xml_->get_widget("credits", credits);
	xml_->get_widget("license", license);
	std::ostringstream vs;
	vs << controller().getVersion()
	   << std::endl << lyx_version_info;
	version->set_text(Glib::locale_to_utf8(vs.str()));
	std::ostringstream crs;
	controller().getCredits(crs);
	credits->set_markup(
		translateMarkup(Glib::convert(crs.str(),
					      "UTF-8",
					      "ISO8859-1")));
	std::ostringstream ls;
	ls << controller().getCopyright() << "\n\n"
	   << controller().getLicense() << "\n\n"
	   << controller().getDisclaimer();
	license->set_text(Glib::locale_to_utf8(ls.str()));
	Gtk::Button * btn;
	xml_->get_widget("close_button", btn);
	setCancel(btn);
	//btn->signal_clicked().connect(SigC::slot(*this, &GViewBase::onCancel));
}
