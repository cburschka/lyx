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

using std::ostringstream;


void MathDimInset::metricsT(TextMetricsInfo const &) const
{
#ifndef WITH_WARNINGS
#warning temporarily disabled
#endif
/*
	std::ostringstream os;
	os << MathAtom(this);
	dim_.wid = int(os.str().size());
	dim_.asc = 1;
	dim_.des = 0;
*/
}


void MathDimInset::drawT(TextPainter &, int, int) const
{
/*
	std::ostringstream os;
	os << MathAtom(this);
	pain.draw(x, y, STRCONV(os.str()));
*/
}
