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
		    lyx::pit_type par,
		    lyx::pos_type pos,
		    int flag);

///
void breakParagraphConservative(BufferParams const & bparams,
				ParagraphList & paragraphs,
				lyx::pit_type par,
				lyx::pos_type pos);

/**
 * Append the next paragraph onto the tail of this one.
 * Be careful, this doesent make any check at all.
 */
void mergeParagraph(BufferParams const & bparams,
	ParagraphList & paragraphs, lyx::pit_type par);


/// for the environments
lyx::pit_type depthHook(lyx::pit_type par,
	ParagraphList const & plist, lyx::depth_type depth);

lyx::pit_type outerHook(lyx::pit_type par, ParagraphList const & plist);

/// Is it the first par with same depth and layout?
bool isFirstInSequence(lyx::pit_type par, ParagraphList const & plist);

/** Check if the current paragraph is the last paragraph in a
    proof environment */
int getEndLabel(lyx::pit_type par, ParagraphList const & plist);

LyXFont const outerFont(lyx::pit_type par_offset, ParagraphList const & pars);

/// return the number of InsetOptArg in a paragraph
int numberOfOptArgs(Paragraph const & par);

#endif // PARAGRAPH_FUNCS_H
