// -*- C++ -*-
/**
 * \file sgml.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SGML_H
#define SGML_H

#include "support/types.h"

#include <iosfwd>
#include <utility>

class Buffer;

namespace sgml {

/**
 * Escape the given character, if necessary,
 * to an SGML entity. Returns true
 * if it was a whitespace character.
 */
std::pair<bool, std::string> escapeChar(char c);

/// Escape a word instead of a single character
std::string escapeString(std::string const & raw);

/// Opens tag
int openTag(Buffer const & buf, std::ostream & os, lyx::depth_type depth,
	    bool mixcont, std::string const & name,
	    std::string const & param = std::string());

/// Close tag
int closeTag(std::ostream & os, lyx::depth_type depth,
	    bool mixcont, std::string const & name);

}
#endif // SGML_H
