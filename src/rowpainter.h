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

class LyXText;
class BufferView;
class VSpace;

/// return the pixel height of a space marker before/after a par
int getLengthMarkerHeight(BufferView const & bv, VSpace const & vsp);

/// paint the rows of the main text, return last drawn y value
int paintText(BufferView const & bv);

/// paint the rows of a text inset
void paintTextInset(BufferView const & bv, LyXText const & text, int x, int y);

#endif // ROWPAINTER_H
