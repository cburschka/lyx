/**
 * \file Alert_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include "frontends/Alert.h"
#include "frontends/Alert_pimpl.h"


namespace {


string translateShortcut(string const & str)
{
	string::size_type i = str.find_first_of("&");
	if (i == string::npos || i == str.length() - 1)
		return str;
	string tstr = str;
	tstr[i] = '_';
	return tstr;
}


}


void warning_pimpl(string const &, string const & message)
{
	Gtk::MessageDialog dlg(Glib::locale_to_utf8(message),
			       Gtk::MESSAGE_WARNING,
			       Gtk::BUTTONS_CLOSE, true, true);
	dlg.run();
}


void error_pimpl(string const &, string const & message)
{
	Gtk::MessageDialog dlg(Glib::locale_to_utf8(message),
			       Gtk::MESSAGE_ERROR,
			       Gtk::BUTTONS_CLOSE, true, true);
	dlg.run();
}


void information_pimpl(string const &, string const & message)
{
	Gtk::MessageDialog dlg(Glib::locale_to_utf8(message),
			       Gtk::MESSAGE_INFO,
			       Gtk::BUTTONS_CLOSE, true, true);
	dlg.run();
}


int prompt_pimpl(string const &, string const & question,
		 int defaultButton, int /*escapeButton*/,
		 string const & b1, string const & b2, string const & b3)
{
	Glib::ustring gb1 = Glib::locale_to_utf8(translateShortcut(b1));
	Glib::ustring gb2 = Glib::locale_to_utf8(translateShortcut(b2));
	Glib::ustring gb3;
	if (!b3.empty())
		gb3 = Glib::locale_to_utf8(translateShortcut(b3));
	Gtk::MessageDialog dlg(Glib::locale_to_utf8(question),
			       Gtk::MESSAGE_QUESTION,
			       Gtk::BUTTONS_NONE, true, true);
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
	Gtk::MessageDialog dlg(Glib::locale_to_utf8(msg),
			       Gtk::MESSAGE_QUESTION,
			       Gtk::BUTTONS_OK_CANCEL,
			       true, true);
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
