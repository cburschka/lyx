/**
 * \file xforms/Alert_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Alert.h"
#include "Alert_pimpl.h"
#include "forms_gettext.h"
#include "gettext.h"
#include "xforms_helpers.h"

#include "debug.h"

#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include "lyx_forms.h"

using std::pair;
using std::make_pair;
using std::endl;

void warning_pimpl(string const &, string const & message)
{
	fl_show_messages(formatted(message, 300).c_str());
}


void error_pimpl(string const &, string const & message)
{
	fl_show_messages(formatted(message, 300).c_str());
}


void information_pimpl(string const &, string const & message)
{
	fl_show_messages(formatted(message, 300).c_str());
}


int prompt_pimpl(string const &, string const & question,
           int default_button, int /*escape_button*/,
	   string const & b1, string const & b2, string const & b3)
{
	string b1label, b1sc;
	string b2label, b2sc;
	string b3label, b3sc;
	boost::tie(b1label, b1sc) = parse_shortcut(b1);
	boost::tie(b2label, b2sc) = parse_shortcut(b2);
	boost::tie(b3label, b3sc) = parse_shortcut(b3);

	if (b3.empty()) {
		fl_set_choices_shortcut(b1sc.c_str(), b2sc.c_str(), "");
		return fl_show_choices(formatted(question, 300).c_str(),
			2, b1label.c_str(), b2label.c_str(), "", default_button + 1) - 1;
	} else {
		fl_set_choices_shortcut(b1sc.c_str(), b2sc.c_str(), b3sc.c_str());
		return fl_show_choices(formatted(question, 300).c_str(),
			3, b1label.c_str(), b2label.c_str(), b3label.c_str(),
			default_button + 1) - 1;
	}
}


pair<bool, string> const askForText_pimpl(string const & msg, string const & dflt)
{
	fl_set_resource("flInput.cancel.label", idex(_("Cancel|^[")).c_str());
	fl_set_resource("flInput.ok.label", idex(_("OK|^M")).c_str());
	fl_set_resource("flInput.clear.label", idex(_("Clear|#C")).c_str());
	char const * tmp = fl_show_input(msg.c_str(), dflt.c_str());
	if (tmp != 0)
		return make_pair<bool, string>(true, string(tmp));
	else
		return make_pair<bool, string>(false, string());
}
