// -*- C++ -*-
/**
 * \file paragraph_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef PARAGRAPH_FUNCS_H
#define PARAGRAPH_FUNCS_H

#include "ParagraphList.h"
#include "support/types.h"

class Buffer;
class BufferParams;
class Paragraph;
class TexRow;

///
void breakParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    ParagraphList::iterator par,
		    lyx::pos_type pos,
		    int flag);

///
void breakParagraphConservative(BufferParams const & bparams,
				ParagraphList & paragraphs,
				ParagraphList::iterator par,
				lyx::pos_type pos);

/**
 * Append the next paragraph onto the tail of this one.
 * Be careful, this doesent make any check at all.
 */
void mergeParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    ParagraphList::iterator par);


#if 0
/// for the environments
Paragraph * depthHook(Paragraph * par, Paragraph::depth_type depth);

Paragraph * outerHook(Paragraph * par);

/// Is it the first par with same depth and layout?
bool isFirstInSequence(Paragraph * par);

/** Check if the current paragraph is the last paragraph in a
    proof environment */
int getEndLabel(Paragraph * para, BufferParams const & bparams);
#endif

ParagraphList::iterator
TeXDeeper(Buffer const * buf,
	  BufferParams const & bparams,
	  ParagraphList const & paragraphs,
	  ParagraphList::iterator pit,
	  std::ostream & os, TexRow & texrow);

ParagraphList::iterator
TeXEnvironment(Buffer const * buf,
	       BufferParams const & bparams,
	       ParagraphList const & paragraphs,
	       ParagraphList::iterator pit,
	       std::ostream & os, TexRow & texrow);

ParagraphList::iterator
TeXOnePar(Buffer const * buf,
	  BufferParams const & bparams,
	  ParagraphList const & paragraphs,
	  ParagraphList::iterator pit,
	  std::ostream & os, TexRow & texrow,
	  bool moving_arg);

#endif // PARAGRAPH_FUNCS_H
