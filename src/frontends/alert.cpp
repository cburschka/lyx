/**
 * \file alert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "alert.h"
#include "Alert_pimpl.h"

#include "debug.h"
#include "LyX.h" // for lyx::use_gui

using std::endl;
using std::string;


namespace lyx {
namespace frontend {

int Alert::prompt(docstring const & title, docstring const & question,
		  int default_button, int escape_button,
		  docstring const & b1, docstring const & b2, docstring const & b3)
{
	if (!use_gui || lyxerr.debugging()) {
		lyxerr << to_utf8(title) << '\n'
		       << "----------------------------------------\n"
		       << to_utf8(question) << endl;

		lyxerr << "Assuming answer is ";
		switch (default_button) {
		case 0: lyxerr << to_utf8(b1) << endl;
		case 1: lyxerr << to_utf8(b2) << endl;
		case 2: lyxerr << to_utf8(b3) << endl;
		}
		if (!use_gui)
			return default_button;
	}

	return prompt_pimpl(title, question,
			    default_button, escape_button, b1, b2, b3);

}


void Alert::warning(docstring const & title, docstring const & message)
{
	lyxerr << "Warning: " << to_utf8(title) << '\n'
	       << "----------------------------------------\n"
	       << to_utf8(message) << endl;

	if (use_gui)
		warning_pimpl(title, message);
}


void Alert::error(docstring const & title, docstring const & message)
{
	lyxerr << "Error: " << to_utf8(title) << '\n'
	       << "----------------------------------------\n"
	       << to_utf8(message) << endl;

	if (use_gui)
		error_pimpl(title, message);
}


void Alert::information(docstring const & title, docstring const & message)
{
	if (!use_gui || lyxerr.debugging())
		lyxerr << to_utf8(title) << '\n'
		       << "----------------------------------------\n"
		       << to_utf8(message) << endl;

	if (use_gui)
		information_pimpl(title, message);
}


bool Alert::askForText(docstring & response, docstring const & msg,
	docstring const & dflt)
{
	if (!use_gui || lyxerr.debugging()) {
		lyxerr << "----------------------------------------\n"
		       << to_utf8(msg) << '\n'
		       << "Assuming answer is " << to_utf8(dflt) << '\n'
		       << "----------------------------------------" << endl;
		if (!use_gui) {
			response = dflt;
			return true;
		}
	}

	return askForText_pimpl(response, msg, dflt);
}

} // namespace frontend
} // namespace lyx
