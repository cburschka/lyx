// -*- C++ -*-
/**
 * \file Alert_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include "debug.h"

// GUI-specific implementations

int prompt_pimpl(string const & title, string const & question,
           int default_button, int escape_button,
	   string const & b1, string const & b2, string const & b3);

void warning_pimpl(string const & title, string const & warning);
void error_pimpl(string const & title, string const & warning);
void information_pimpl(string const & title, string const & warning);

std::pair<bool, string> const askForText_pimpl(string const & msg, string const & dflt);
