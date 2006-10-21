/**
 * \file lcolorcache.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lcolorcache.h"

#include "Color.h"
#include "LColor.h"


namespace lyx {

LColorCache lcolorcache;


LColorCache::LColorCache()
{
}


QColor const & LColorCache::get(LColor_color col) const
{
	lcolor_map::const_iterator cit = colormap.find(col);
	if (cit != colormap.end())
		return cit->second;

	QColor const qcol(lcolor.getX11Name(col).c_str());
	colormap[col] = qcol;
	return colormap[col];
}


void LColorCache::clear()
{
	colormap.clear();
}


QColor const rgb2qcolor(lyx::RGBColor const & rgb)
{
	return QColor(rgb.r, rgb.g, rgb.b);
}


} // namespace lyx
