// -*- C++ -*-
/**
 * \file paragraph_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_FUNCS_H
#define PARAGRAPH_FUNCS_H

#include "ParagraphList_fwd.h"
#include "support/types.h"

#include <string>

class Buffer;
class BufferParams;
class LyXFont;
class LyXLex;
class InsetBase;


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
ParagraphList::iterator depthHook(ParagraphList::iterator pit,
				  ParagraphList const & plist,
				  lyx::depth_type depth);

ParagraphList::iterator outerHook(ParagraphList::iterator pit,
				  ParagraphList const & plist);

/// Is it the first par with same depth and layout?
bool isFirstInSequence(ParagraphList::iterator par,
		       ParagraphList const & plist);

/** Check if the current paragraph is the last paragraph in a
    proof environment */
int getEndLabel(ParagraphList::iterator pit,
		ParagraphList const & plist);

/// read a paragraph from a .lyx file. Returns number of unrecognised tokens
int readParagraph(Buffer const & buf, Paragraph & par, LyXLex & lex);

LyXFont const outerFont(ParagraphList::iterator pit,
			ParagraphList const & plist);

/// find outermost paragraph containing an inset
ParagraphList::iterator outerPar(Buffer const & buf, InsetBase const * inset);

/// find owning paragraph containing an inset
Paragraph const & ownerPar(Buffer const & buf, InsetBase const * inset);

/// return the range of pars [beg, end[ owning the range of y [ystart, yend] 
void getParsInRange(ParagraphList & pl,
				int ystart, int yend,
				ParagraphList::iterator & beg,
				ParagraphList::iterator & end);

#endif // PARAGRAPH_FUNCS_H
