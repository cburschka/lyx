// -*- C++ -*-
/**
 * \file output_docbook.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author José Matos
 * \author Thibaut Cuvelier
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_DOCBOOK_H
#define OUTPUT_DOCBOOK_H

#include "LayoutEnums.h"
#include "ParagraphList.h"

#include "support/docstream.h"
#include "support/strfwd.h"
#include "xml.h"

namespace lyx {

class Buffer;
class OutputParams;
class Text;

/// generates an xml::StartTag for the given style
xml::FontTag docbookStartFontTag(xml::FontTypes type);
/// generates an xml::EndTag for the given style
xml::EndFontTag docbookEndFontTag(xml::FontTypes type);

/// output a series of paragraphs into the XMLStream
void docbookParagraphs(Text const &,
		               Buffer const &,
                       XMLStream &,
		               OutputParams const &);

/// output a single paragraph into the XMLStream
ParagraphList::const_iterator makeAny(Text const &,
                                      Buffer const &,
                                      XMLStream &,
                                      OutputParams const &,
                                      ParagraphList::const_iterator);

} // namespace lyx

#endif
