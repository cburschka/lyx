/**
 * \file xforms/Alert_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "Alert.h"
#include "Alert_pimpl.h"
#include "forms_gettext.h"
#include "gettext.h"

#include <algorithm>
#include FORMS_H_LOCATION

using std::pair;
using std::make_pair;

void alert_pimpl(string const & s1, string const & s2, string const & s3)
{
	fl_set_resource("flAlert.dismiss.label", _("Dismiss"));
	fl_show_alert(s1.c_str(), s2.c_str(), s3.c_str(), 0);
}


bool askQuestion_pimpl(string const & s1, string const & s2, string const & s3)
{
	fl_set_resource("flQuestion.yes.label", idex(_("Yes|#y")).c_str());
	fl_set_resource("flQuestion.no.label", idex(_("No|#n")).c_str());
	return fl_show_question((s1 + "\n" + s2 + "\n" + s3).c_str(), 0);
}


int askConfirmation_pimpl(string const & s1, string const & s2, string const & s3)
{
	string const yes    = _("Yes|#Y");
	string const no     = _("No|#N");
	string const cancel = _("Cancel|^[");
	
	fl_set_choices_shortcut(scex(yes).c_str(), scex(no).c_str(),
				scex(cancel).c_str());
	return fl_show_choice(s1.c_str(), s2.c_str(), s3.c_str(),
			      3, idex(yes).c_str(), idex(no).c_str(),
			      idex(cancel).c_str(), 3);
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
