// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef PARAGRAPH_FUNCS_H
#define PARAGRAPH_FUNCS_H

#include "ParagraphList.h"
#include "paragraph.h"

#include "support/types.h"

class BufferParams;

///
void breakParagraph(BufferParams const & bparams,
		    Paragraph *,
		    lyx::pos_type pos,
		    int flag);

///
void breakParagraphConservative(BufferParams const & bparams,
				Paragraph *,
				lyx::pos_type pos);

/** Paste this paragraph with the next one.
    Be carefull, this doesent make any check at all.
*/
void pasteParagraph(BufferParams const & bparams,
		    Paragraph *);


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

#endif
