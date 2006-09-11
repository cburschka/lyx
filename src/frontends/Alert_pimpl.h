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

// GUI-specific implementations

#include "support/docstring.h"

#include <utility>
#include <string>


int prompt_pimpl(lyx::docstring const & title, lyx::docstring const & question,
		 int default_button, int escape_button,
		 lyx::docstring const & b1,
		 lyx::docstring const & b2,
		 lyx::docstring const & b3);

void warning_pimpl(lyx::docstring const & title, lyx::docstring const & warning);
void error_pimpl(lyx::docstring const & title, lyx::docstring const & warning);
void information_pimpl(lyx::docstring const & title, lyx::docstring const & warning);

std::pair<bool, lyx::docstring> const askForText_pimpl(lyx::docstring const & msg, lyx::docstring const & dflt);
