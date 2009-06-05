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
class ParagraphList;

///
void xhtmlParagraphs(ParagraphList const & subset,
		       Buffer const & buf,
		       odocstream & os,
		       OutputParams const & runparams);

namespace html {
	docstring escapeChar(char_type c);
	void openTag(odocstream & os, std::string tag, std::string attr);
	void closeTag(odocstream & os, std::string tag);
}
} // namespace lyx

#endif
