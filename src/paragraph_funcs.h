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

#include "support/types.h"

class Buffer;
class BufferParams;
class InsetBase;
class LyXFont;
class Paragraph;
class ParagraphList;


///
void breakParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    lyx::par_type par,
		    lyx::pos_type pos,
		    int flag);

///
void breakParagraphConservative(BufferParams const & bparams,
				ParagraphList & paragraphs,
				lyx::par_type par,
				lyx::pos_type pos);

/**
 * Append the next paragraph onto the tail of this one.
 * Be careful, this doesent make any check at all.
 */
void mergeParagraph(BufferParams const & bparams,
	ParagraphList & paragraphs, lyx::par_type par);


/// for the environments
lyx::par_type depthHook(lyx::par_type par,
	ParagraphList const & plist, lyx::depth_type depth);

lyx::par_type outerHook(lyx::par_type par, ParagraphList const & plist);

/// Is it the first par with same depth and layout?
bool isFirstInSequence(lyx::par_type par, ParagraphList const & plist);

/** Check if the current paragraph is the last paragraph in a
    proof environment */
int getEndLabel(lyx::par_type par, ParagraphList const & plist);

LyXFont const outerFont(lyx::par_type par, ParagraphList const & plist);

/// find outermost paragraph containing an inset
lyx::par_type outerPar(Buffer const & buf, InsetBase const * inset);

/// return the range of pars [beg, end[ owning the range of y [ystart, yend]
void getParsInRange(ParagraphList & plist,
				int ystart, int yend,
				lyx::par_type & beg,
				lyx::par_type & end);

#endif // PARAGRAPH_FUNCS_H
