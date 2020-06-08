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

#include "support/docstream.h"
#include "support/strfwd.h"
#include "xml.h"

namespace lyx {

class Buffer;
class OutputParams;
class Text;

///
std::string const fontToDocBookTag(xml::FontTypes type);
///
xml::FontTag docbookStartFontTag(xml::FontTypes type);
///
xml::EndFontTag docbookEndFontTag(xml::FontTypes type);

///
void docbookParagraphs(Text const & text,
		               Buffer const & buf,
                       XMLStream & os,
		               OutputParams const & runparams);

} // namespace lyx

#endif
