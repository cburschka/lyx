// -*- C++ -*-
/**
 * \file rowpainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ROWPAINTER_H
#define ROWPAINTER_H

#include "support/types.h"

class LyXText;
class BufferView;
class PainterInfo;

/// paint the rows of the main text, return last drawn y value
int paintText(BufferView const & bv);

/// refresh a par of the main text
void refreshPar(BufferView const & bv, LyXText const & text,
		lyx::par_type pit);

/// paint the rows of a text inset
void paintTextInset(LyXText const & text, PainterInfo & pi);

#endif // ROWPAINTER_H
