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
#include "debug.h"


MathDimInset::MathDimInset()
	: xo_(-3), yo_(-3)
{}


int MathDimInset::ascent() const
{
	return dim_.asc;
}


int MathDimInset::descent() const
{
	return dim_.des;
}


int MathDimInset::width() const
{
	return dim_.wid;
}


void MathDimInset::setPosCache(PainterInfo const &, int x, int y) const
{
	lyxerr << "MathDimInset:: position cache to " << x << " " << y << std::endl;
	xo_ = x;
	yo_ = y;
}
