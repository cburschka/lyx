/**
 * \file sgml.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author José Matos
 * \author John Levon <levon@movementarian.org>
 */

#ifndef SGML_H
#define SGML_H
 
#include "LString.h"
 
#include <algorithm>
#include <iosfwd>
 
namespace sgml {

/**
 * Escape the given character if necessary
 * to an SGML entity. Returns true
 * if it was a whitespace character.
 */
std::pair<bool, string> escapeChar(char c);

/// FIXME
int openTag(std::ostream & os, Paragraph::depth_type depth,
	    bool mixcont, string const & latexname);

/// FIXME
int closeTag(std::ostream & os, Paragraph::depth_type depth,
	    bool mixcont, string const & latexname);
}

#endif // SGML_H
