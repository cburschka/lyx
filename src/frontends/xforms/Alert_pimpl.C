/**
 * \file xforms/Alert_pimpl.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Alert.h"
#include "Alert_pimpl.h" 
#include "xforms_helpers.h"
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
	fl_set_resource("flQuestion.yes.label", idex(_("Yes|Yy#y")));
	fl_set_resource("flQuestion.no.label", idex(_("No|Nn#n")));
	return fl_show_question((s1 + "\n" + s2 + "\n" + s3).c_str(), 0);
}


int askConfirmation_pimpl(string const & s1, string const & s2, string const & s3)
{
	fl_set_choices_shortcut(scex(_("Yes|Yy#y")),
				scex(_("No|Nn#n")),
				scex(_("Cancel|^[")));
        return fl_show_choice(s1.c_str(), s2.c_str(), s3.c_str(), 
			      3, idex(_("Yes|Yy#y")),
			      idex(_("No|Nn#n")),
                              idex(_("Cancel|^[")), 3);
}


pair<bool, string> const askForText_pimpl(string const & msg, string const & dflt)
{
	fl_set_resource("flInput.cancel.label", idex(_("Cancel|^[")));
	fl_set_resource("flInput.ok.label", idex(_("OK|#O")));
	fl_set_resource("flInput.clear.label", idex(_("Clear|#e")));
	char const * tmp = fl_show_input(msg.c_str(), dflt.c_str());
	if (tmp != 0)
	  return make_pair<bool, string>(true, string(tmp));
	else
	  return make_pair<bool, string>(false, string());
}
