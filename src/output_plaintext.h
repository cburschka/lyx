// -*- C++ -*-
/**
 * \file output_plaintext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_PLAINTEXT_H
#define OUTPUT_PLAINTEXT_H

#include "support/docstream.h"


namespace lyx {

namespace support { class FileName; }

class Buffer;
class OutputParams;
class Paragraph;


///
void writeFileAscii(Buffer const & buf, support::FileName const &,
	OutputParams const &);

///
void writeFileAscii(Buffer const & buf, odocstream &, OutputParams const &);

///
void asciiParagraph(Buffer const & buf,
		    Paragraph const & paragraphs,
		    odocstream & ofs,
		    OutputParams const &,
		    bool & ref_printed);

} // namespace lyx

#endif
