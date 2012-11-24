// -*- C++ -*-
/**
 * \file output_latex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_LATEX_H
#define OUTPUT_LATEX_H

#include <utility>

#include "support/docstream.h"
#include "Layout.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "ParagraphList.h"


namespace lyx {

class Buffer;
class BufferParams;
class Encoding;
class Layout;
class Paragraph;
class OutputParams;
class TexRow;
class Text;

/// Export up to \p reqargs required arguments and
/// \p optargs optional ones. If not enough required
/// ones are given, we'll output: {}. The optional ones
/// must all come first.
void latexArgInsets(Paragraph const & par,
		otexstream & os, OutputParams const & runparams,
		Layout::LaTeXArgMap const & latexargs);

/** Export \p paragraphs of buffer \p buf to LaTeX.
    Don't use a temporary stringstream for \p os if the final output is
    supposed to go to a file.
    \sa Buffer::writeLaTeXSource for the reason.
 */
void latexParagraphs(Buffer const & buf,
		     Text const & text,
		     otexstream & ofs,
		     OutputParams const &,
		     std::string const & everypar = std::string());

/** Switch the encoding of \p os from runparams.encoding to \p newEnc if needed.
    \p force forces this also within non-default or -auto encodings.
    \return (did the encoding change?, number of characters written to \p os)
 */
std::pair<bool, int> switchEncoding(odocstream & os,
		     BufferParams const & bparams,
		     OutputParams const &, Encoding const & newEnc,
		     bool force = false);

/// FIXME: this should not be visible.
void TeXOnePar(Buffer const & buf,
	           Text const & text,
	           pit_type pit,
	           otexstream & os,
	           OutputParams const & runparams,
	           std::string const & everypar = std::string(),
	           int start_pos = -1, int end_pos = -1);

} // namespace lyx

#endif
