/**
 * \file gtk/Alert_pimpl.C
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

#include "frontends/Alert.h"
#include "frontends/Alert_pimpl.h"

#include <gtkmm.h>

using lyx::docstring;

using std::string;


namespace {


docstring translateShortcut(docstring const & str)
{
	docstring::size_type i = str.find_first_of(lyx::from_ascii("&"));
	if (i == docstring::npos || i == str.length() - 1)
		return str;
	docstring tstr = str;
	tstr[i] = '_';
	return tstr;
}


}


void warning_pimpl(docstring const & title, docstring const & message)
{
	Gtk::MessageDialog dlg(Glib::Markup::escape_text(lyx::to_utf8(message)),
			       true, Gtk::MESSAGE_WARNING,
			       Gtk::BUTTONS_CLOSE, true);
	dlg.set_title(lyx::to_utf8(title));
	dlg.run();
}


void error_pimpl(docstring const & title, docstring const & message)
{
	Gtk::MessageDialog dlg(Glib::Markup::escape_text(lyx::to_utf8(message)),
			       true, Gtk::MESSAGE_ERROR,
			       Gtk::BUTTONS_CLOSE, true);
	dlg.set_title(lyx::to_utf8(title));
	dlg.run();
}


void information_pimpl(docstring const & title, docstring const & message)
{
	Gtk::MessageDialog dlg(Glib::Markup::escape_text(lyx::to_utf8(message)),
			       true, Gtk::MESSAGE_INFO,
			       Gtk::BUTTONS_CLOSE, true);
	dlg.set_title(lyx::to_utf8(title));
	dlg.run();
}


int prompt_pimpl(docstring const & title, docstring const & question,
		 int defaultButton, int /*escapeButton*/,
		 docstring const & b1, docstring const & b2, docstring const & b3)
{
	Glib::ustring gb1 = lyx::to_utf8(translateShortcut(b1));
	Glib::ustring gb2 = lyx::to_utf8(translateShortcut(b2));
	Glib::ustring gb3;
	if (!b3.empty())
		gb3 = lyx::to_utf8(translateShortcut(b3));
	Gtk::MessageDialog dlg(Glib::Markup::escape_text(lyx::to_utf8(question)),
			       true, Gtk::MESSAGE_QUESTION,
			       Gtk::BUTTONS_NONE, true);
	dlg.set_title(lyx::to_utf8(title));
	dlg.add_button(gb1, 0);
	dlg.add_button(gb2, 1);
	if (!b3.empty())
		dlg.add_button(gb3, 2);
	dlg.set_default_response(defaultButton);
	return dlg.run();
}


std::pair<bool, string> const askForText_pimpl(string const & msg,
					       string const & dflt)
{
	Gtk::MessageDialog dlg(Glib::Markup::escape_text(Glib::locale_to_utf8(msg)),
			       true, Gtk::MESSAGE_QUESTION,
			       Gtk::BUTTONS_OK_CANCEL,
			       true);
	Gtk::Entry entry;
	entry.set_text(Glib::locale_to_utf8(dflt));
	entry.set_position(-1);
	entry.show();
	dlg.get_vbox()->children().push_back(
		Gtk::Box_Helpers::Element(entry));
	int response = dlg.run();
	if (response == Gtk::RESPONSE_OK) {
		string str = Glib::locale_from_utf8(entry.get_text());
		return std::make_pair<bool, string>(true, str);
	}
	else
		return std::make_pair<bool, string>(false, string());
}
