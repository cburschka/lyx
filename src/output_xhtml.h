// -*- C++ -*-
/**
 * \file output_xhtml.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_XHTML_H
#define OUTPUT_XHTML_H

#include "support/strfwd.h"

namespace lyx {

class Buffer;
class OutputParams;
class Text;

///
void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       odocstream & os,
		       OutputParams const & runparams);

namespace html {
	///
	docstring escapeChar(char_type c);
	/// converts a string to a form safe for links, etc
	docstring htmlize(docstring const & str);
	/// \return true if tag was opened, false if not 
	bool openTag(odocstream & os, std::string const & tag, 
	             std::string const & attr);
	/// \return true if tag was opened, false if not 
	bool closeTag(odocstream & os, std::string const & tag);
}
} // namespace lyx

#endif
