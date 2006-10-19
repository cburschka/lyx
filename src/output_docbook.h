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

#include "ParagraphList_fwd.h"
#include "support/docstream.h"

#include <iosfwd>

class Buffer;
class OutputParams;

///
void docbookParagraphs(ParagraphList const & subset,
		       Buffer const & buf,
		       lyx::odocstream & os,
		       OutputParams const & runparams);
#endif
