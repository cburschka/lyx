/**
 * \file math_diminset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_diminset.h"
#include "coordcache.h"
#include "debug.h"


InsetMathDim::InsetMathDim()
{}


int InsetMathDim::ascent() const
{
	return dim_.asc;
}


int InsetMathDim::descent() const
{
	return dim_.des;
}


int InsetMathDim::width() const
{
	return dim_.wid;
}


void InsetMathDim::setPosCache(PainterInfo const &, int x, int y) const
{
	//lyxerr << "InsetMathDim: cache to " << x << " " << y << std::endl;
	theCoords.insets().add(this, x, y);
}
