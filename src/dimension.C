/**
 * \file dimension.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */


#include <config.h>

#include "dimension.h"
#include "frontends/font_metrics.h"


void Dimension::operator+=(Dimension const & dim)
{
	if (a < dim.a)
		a = dim.a;
	if (d < dim.d)
		d = dim.d;
	w += dim.w;
}


void Dimension::clear(LyXFont const & font)
{
	a = font_metrics::maxAscent(font);
	d = font_metrics::maxDescent(font);
	w = 0;
}
