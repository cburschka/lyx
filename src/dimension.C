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
	if (asc < dim.asc)
		asc = dim.asc;
	if (des < dim.des)
		des = dim.des;
	wid += dim.wid;
}


void Dimension::clear(LyXFont const & font)
{
	asc = font_metrics::maxAscent(font);
	des = font_metrics::maxDescent(font);
	wid = 0;
}
