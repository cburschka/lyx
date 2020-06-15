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

#include "LayoutEnums.h"

#include "support/docstream.h"
#include "support/strfwd.h"
#include "xml.h"

#include <deque>
#include <memory>


namespace lyx {

docstring const fontToHtmlTag(xml::FontTypes type);

class Buffer;
class OutputParams;
class Text;

///
xml::FontTag xhtmlStartFontTag(xml::FontTypes type);
///
xml::EndFontTag xhtmlEndFontTag(xml::FontTypes type);

///
void xhtmlParagraphs(Text const & text,
		       Buffer const & buf,
		       XMLStream & xs,
		       OutputParams const & runparams);

/// \return a string appropriate for setting alignment in CSS
/// Does NOT return "justify" for "block"
std::string alignmentToCSS(LyXAlignment align);

} // namespace lyx

#endif
