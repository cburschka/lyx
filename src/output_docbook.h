// -*- C++ -*-
/**
 * \file output_docbook.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author José Matos
 * \author Thibaut Cuvelier
 * \author Richard Kimberly Heck
 *
 * Full author contact details are available in file CREDITS.
 */

// TODO: respect languages when opening tags: par.getParLanguage(bparams) in output_latex.cpp
// TODO: if the user specifically asks, output change-tracking information (choice between revisionflag attribute and
//  Oxygen's formatting)
//    <?oxy_options track_changes="on"?> <!-- After closing root tag -->
//    <?oxy_comment_start author="XXX" timestamp="20221226T235332+0100" comment="XXX"?>...<?oxy_comment_end?>
//    <?oxy_insert_start author="XXX" timestamp="20221226T235458+0100"?>XXX<?oxy_insert_end?>
//    <?oxy_delete author="XXX" timestamp="20221226T235519+0100" content="XXX"?> <!-- XML-encoded deleted text
//    (including tags) -->

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
