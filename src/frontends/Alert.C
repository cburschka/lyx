/**
 * \file Alert.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Alert.h"

#include "debug.h"
#include "lyx_gui.h"

#include "Alert_pimpl.h"

using std::endl;
using std::pair;
using std::make_pair;

int Alert::prompt(string const & title, string const & question,
           int default_button, int escape_button,
	   string const & b1, string const & b2, string const & b3)
{
	if (!lyx_gui::use_gui || lyxerr.debugging()) {
		lyxerr << title
		       << "----------------------------------------"
		       << question << endl;

		lyxerr << "Assuming answer is ";
		switch (default_button) {
		case 0: lyxerr << b1 << endl;
		case 1: lyxerr << b2 << endl;
		case 2: lyxerr << b3 << endl;
		}
		if (!lyx_gui::use_gui)
			return default_button;
	}

	return prompt_pimpl(title, question,
			    default_button, escape_button, b1, b2, b3);

}


void Alert::warning(string const & title, string const & message)
{
	if (!lyx_gui::use_gui || lyxerr.debugging())
		lyxerr << "Warning: " << title
		       << "----------------------------------------"
		       << message << endl;
	if (lyx_gui::use_gui)
		warning_pimpl(title, message);
}


void Alert::error(string const & title, string const & message)
{
	if (!lyx_gui::use_gui || lyxerr.debugging())
		lyxerr << "Error: " << title << '\n'
		       << "----------------------------------------\n"
		       << message << endl;

	if (lyx_gui::use_gui)
		error_pimpl(title, message);
}


void Alert::information(string const & title, string const & message)
{
	if (!lyx_gui::use_gui || lyxerr.debugging())
		lyxerr << title
		       << "----------------------------------------"
		       << message << endl;

	if (lyx_gui::use_gui)
		information_pimpl(title, message);
}


pair<bool, string> const Alert::askForText(string const & msg,
					   string const & dflt)
{
	if (!lyx_gui::use_gui || lyxerr.debugging()) {
		lyxerr << "----------------------------------------\n"
		       << msg << '\n'
		       << "Assuming answer is " << dflt << '\n'
		       << "----------------------------------------" << endl;
		if (!lyx_gui::use_gui)
			return make_pair<bool, string>(true, dflt);
	}

	return askForText_pimpl(msg, dflt);
}
