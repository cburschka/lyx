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
#include "Alert_pimpl.h"

#include "debug.h"
#include "lyx_gui.h"

using lyx::docstring;

using std::endl;
using std::make_pair;
using std::pair;
using std::string;


namespace lyx {
namespace frontend {

int Alert::prompt(docstring const & title, docstring const & question,
		  int default_button, int escape_button,
		  docstring const & b1, docstring const & b2, docstring const & b3)
{
	if (!lyx_gui::use_gui || lyxerr.debugging()) {
		lyxerr << lyx::to_utf8(title) << '\n'
		       << "----------------------------------------\n"
		       << lyx::to_utf8(question) << endl;

		lyxerr << "Assuming answer is ";
		switch (default_button) {
		case 0: lyxerr << lyx::to_utf8(b1) << endl;
		case 1: lyxerr << lyx::to_utf8(b2) << endl;
		case 2: lyxerr << lyx::to_utf8(b3) << endl;
		}
		if (!lyx_gui::use_gui)
			return default_button;
	}

	return prompt_pimpl(title, question,
			    default_button, escape_button, b1, b2, b3);

}


void Alert::warning(docstring const & title, docstring const & message)
{
	if (!lyx_gui::use_gui || lyxerr.debugging())
		lyxerr << "Warning: " << lyx::to_utf8(title) << '\n'
		       << "----------------------------------------\n"
		       << lyx::to_utf8(message) << endl;
	if (lyx_gui::use_gui)
		warning_pimpl(title, message);
}


void Alert::error(docstring const & title, docstring const & message)
{
	if (!lyx_gui::use_gui || lyxerr.debugging())
		lyxerr << "Error: " << lyx::to_utf8(title) << '\n'
		       << "----------------------------------------\n"
		       << lyx::to_utf8(message) << endl;

	if (lyx_gui::use_gui)
		error_pimpl(title, message);
}


void Alert::information(docstring const & title, docstring const & message)
{
	if (!lyx_gui::use_gui || lyxerr.debugging())
		lyxerr << lyx::to_utf8(title) << '\n'
		       << "----------------------------------------\n"
		       << lyx::to_utf8(message) << endl;

	if (lyx_gui::use_gui)
		information_pimpl(title, message);
}


pair<bool, docstring> const Alert::askForText(docstring const & msg,
					   docstring const & dflt)
{
	if (!lyx_gui::use_gui || lyxerr.debugging()) {
		lyxerr << "----------------------------------------\n"
		       << lyx::to_utf8(msg) << '\n'
		       << "Assuming answer is " << lyx::to_utf8(dflt) << '\n'
		       << "----------------------------------------" << endl;
		if (!lyx_gui::use_gui)
			return make_pair<bool, docstring>(true, dflt);
	}

	return askForText_pimpl(msg, dflt);
}

}
}
