// -*- C++ -*-
/**
 * \file output_docbook.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_DOCBOOK_H
#define OUTPUT_DOCBOOK_H

#include "support/strfwd.h"
#include "xml.h"

namespace lyx {

std::string const fontToDocBookTag(xml::FontTypes type);

class Buffer;
class OutputParams;
class Text;

///
void docbookParagraphs(Text const & text,
		       Buffer const & buf,
		       odocstream & os,
		       OutputParams const & runparams);

} // namespace lyx

#endif
