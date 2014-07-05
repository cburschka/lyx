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

#include "support/docstring.h"

namespace lyx {

class Buffer;
class Paragraph;
class OutputParams;

namespace sgml {

/**
 * Escape the given character, if necessary,
 * to an SGML entity.
 */
docstring escapeChar(char_type c);

/// Escape a word instead of a single character
docstring escapeString(docstring const & raw);

/// replaces illegal characters from SGML/XML ID attributes
docstring cleanID(Buffer const & buf, OutputParams const & runparams,
		    docstring const & orig);

/// returns a unique numeric id
docstring const uniqueID(docstring const & label);

/// Opens tag
void openTag(odocstream & os, std::string const & name,
	    std::string const & attribute = std::string());

/// Open tag
void openTag(Buffer const & buf, odocstream & os,
	     OutputParams const & runparams, Paragraph const & par);

/// Close tag
void closeTag(odocstream & os, std::string const & name);

/// Close tag
void closeTag(odocstream & os, Paragraph const & par);

} // namespace sgml
} // namespace lyx

#endif // SGML_H
