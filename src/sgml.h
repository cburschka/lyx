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

namespace sgml {

/**
 * Escape the given character if necessary
 * to an SGML entity. Returns true
 * if it was a whitespace character.
 */
std::pair<bool, std::string> escapeChar(char c);

/// FIXME
int openTag(std::ostream & os, lyx::depth_type depth,
	    bool mixcont, std::string const & latexname,
		std::string const & latexparam = std::string());

/// FIXME
int closeTag(std::ostream & os, lyx::depth_type depth,
	    bool mixcont, std::string const & latexname);

///
unsigned int closeEnvTags(std::ostream & os, 
		bool mixcont,
		std::string const & environment_inner_depth,
		std::string const & item_tag,
		lyx::depth_type total_depth);

}

#endif // SGML_H
