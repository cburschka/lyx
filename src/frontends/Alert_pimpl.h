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

#include <utility>
#include <string>


int prompt_pimpl(std::string const & title, std::string const & question,
           int default_button, int escape_button,
	   std::string const & b1, std::string const & b2, std::string const & b3);

void warning_pimpl(std::string const & title, std::string const & warning);
void error_pimpl(std::string const & title, std::string const & warning);
void information_pimpl(std::string const & title, std::string const & warning);

std::pair<bool, std::string> const askForText_pimpl(std::string const & msg, std::string const & dflt);
