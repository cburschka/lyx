// -*- C++ -*-
/**
 * \file rowpainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef ROWPAINTER_H
#define ROWPAINTER_H

#include <config.h>

#include "RowList.h"

class LyXText;
class BufferView;
class VSpace;

/// paint the rows
// return last used y
int paintRows(BufferView const & bv, LyXText const & text,
	ParagraphList::iterator pit,
	RowList::iterator rit, int xo, int y, int yf, int yo);

/// return the pixel height of a space marker before/after a par
int getLengthMarkerHeight(BufferView const & bv, VSpace const & vsp);

#endif // ROWPAINTER_H
