/**
 * \file Alert.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
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


bool Alert::askQuestion(string const & s1, string const & s2,
			string const & s3, bool default_value)
{
	if (!lyxrc.use_gui) {
		lyxerr << "----------------------------------------" << endl
		       << s1 << endl;
		if (!s2.empty())
			lyxerr << s2 << endl;
		if (!s3.empty())
			lyxerr << s3 << endl;
		lyxerr << "Assuming answer is "
		       << (default_value ? "yes" : "no")
		       << endl
		       << "----------------------------------------" << endl;
		return default_value;
	} else {
		return askQuestion_pimpl(s1, s2, s3);
	}
}


int Alert::askConfirmation(string const & s1, string const & s2,
			   string const & s3, int default_value)
{
	if (!lyxrc.use_gui) {
		lyxerr << "----------------------------------------" << endl
		       << s1 << endl;
		if (!s2.empty())
			lyxerr << s2 << endl;
		if (!s3.empty())
			lyxerr << s3 << endl;
		lyxerr << "Assuming answer is ";
		if (default_value == 1)
			lyxerr << "yes";
		else if (default_value == 2)
			lyxerr << "no";
		else
			lyxerr << "cancel";
		lyxerr << endl
		       << "----------------------------------------" << endl;
		return default_value;
	} else {
		return askConfirmation_pimpl(s1, s2, s3);
	}
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
