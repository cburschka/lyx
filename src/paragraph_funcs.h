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
#include "paragraph.h"
#include "support/types.h"

class Buffer;
class BufferParams;
class TexRow;
class LyXLex;

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

/// for the environments
Paragraph * depthHook(Paragraph * par, Paragraph::depth_type depth);

Paragraph * outerHook(Paragraph * par);

/// Is it the first par with same depth and layout?
bool isFirstInSequence(Paragraph * par);

/** Check if the current paragraph is the last paragraph in a
    proof environment */
int getEndLabel(Paragraph * para);


void latexParagraphs(Buffer const * buf,
		     ParagraphList const & paragraphs,
		     ParagraphList::iterator par,
		     ParagraphList::iterator endpar,
		     std::ostream & ofs,
		     TexRow & texrow,
		     bool moving_arg = false);

/// read a paragraph from a .lyx file. Returns number of unrecognised tokens
int readParagraph(Buffer & buf, Paragraph & par, LyXLex & lex);

LyXFont const realizeFont(LyXFont const & font,
			  BufferParams const & params);

LyXFont const outerFont(ParagraphList::iterator pit,
			ParagraphList const & plist);

#endif // PARAGRAPH_FUNCS_H
