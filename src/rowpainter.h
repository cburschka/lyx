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

/// initialise painter and paint the rows
void paintRows2(BufferView const & bv, LyXText const & text,
	RowList::iterator rit, RowList::iterator end,
	int xo, int & y, int yf, int y2, int yo);

/// return the pixel height of a space marker before/after a par
int getLengthMarkerHeight(BufferView const & bv, VSpace const & vsp);

#endif // ROWPAINTER_H
