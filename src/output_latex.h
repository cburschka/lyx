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

#include <string>

class Buffer;
class OutputParams;
class ParagraphList;
class TexRow;

/// Just a wrapper for the method below, first creating the ofstream.

void latexParagraphs(Buffer const & buf,
		     ParagraphList const & paragraphs,
		     std::ostream & ofs,
		     TexRow & texrow,
		     OutputParams const &,
		     std::string const & everypar = std::string());

#endif
