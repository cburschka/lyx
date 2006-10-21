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


namespace lyx {

int prompt_pimpl(docstring const & title, docstring const & question,
		 int default_button, int escape_button,
		 docstring const & b1,
		 docstring const & b2,
		 docstring const & b3);

void warning_pimpl(docstring const & title, docstring const & warning);
void error_pimpl(docstring const & title, docstring const & warning);
void information_pimpl(docstring const & title, docstring const & warning);

std::pair<bool, docstring> const askForText_pimpl(docstring const & msg, docstring const & dflt);

} // namespace lyx

