/**
 * \file Alert.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "Alert.h"

#include "debug.h"
#include "lyxrc.h"

#include "Alert_pimpl.h"

#include <cerrno>

#ifndef CXX_GLOBAL_CSTD
using std::strerror;
#endif

using std::endl;
using std::pair;
using std::make_pair;


void Alert::alert(string const & s1, string const & s2, string const & s3)
{
	if (!lyxrc.use_gui) {
		lyxerr << "------------------------------" << endl
		       << s1 << endl << s2 << endl << s3 << endl
		       << "------------------------------" << endl;
	} else {
		alert_pimpl(s1, s2, s3);
	}
}


void Alert::err_alert(string const & s1, string const & s2)
{
	alert(s1, s2, strerror(errno));
}


int Alert::prompt(string const & title, string const & question,
           int default_button,
	   string const & b1, string const & b2, string const & b3)
{
	if (lyxrc.use_gui)
		return prompt_pimpl(title, question, default_button, b1, b2, b3);

	lyxerr << title << endl;
	lyxerr << "----------------------------------------" << endl;
	lyxerr << question << endl;
	lyxerr << "Assuming answer is ";
	switch (default_button) {
		case 0: lyxerr << b1 << endl;
		case 1: lyxerr << b2 << endl;
		case 2: lyxerr << b3 << endl;
	}
	return default_button;
}


pair<bool, string> const Alert::askForText(string const & msg,
					   string const & dflt)
{
	if (!lyxrc.use_gui) {
		lyxerr << "----------------------------------------" << endl
		       << msg << endl
		       << "Assuming answer is " << dflt
		       << "----------------------------------------" << endl;
		return make_pair<bool, string>(true, dflt);
	} else {
		return askForText_pimpl(msg, dflt);
	}
}
