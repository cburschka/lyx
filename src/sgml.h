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
#include <string>
#include <utility>

class Buffer;
class Paragraph;
class OutputParams;

namespace sgml {

/**
 * Escape the given character, if necessary,
 * to an SGML entity. Returns true
 * if it was a whitespace character.
 */
std::pair<bool, std::string> escapeChar(char c);

/// Escape a word instead of a single character
std::string escapeString(std::string const & raw);

/// replaces illegal characters from SGML/XML ID attributes
std::string cleanID(Buffer const & buf, OutputParams const & runparams,
		    std::string const & orig);

/// returns a unique numeric id
std::string const uniqueID(std::string const label);

/// Opens tag
void openTag(std::ostream & os, std::string const & name,
	    std::string const & attribute = std::string());

/// Open tag
void openTag(Buffer const & buf, std::ostream & os,
	     OutputParams const & runparams, Paragraph const & par);

/// Close tag
void closeTag(std::ostream & os, std::string const & name);

/// Close tag
void closeTag(std::ostream & os, Paragraph const & par);
}
#endif // SGML_H
