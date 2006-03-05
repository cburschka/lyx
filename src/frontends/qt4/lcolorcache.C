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

#include "LColor.h"

LColorCache lcolorcache;


LColorCache::LColorCache()
{
}


QColor const & LColorCache::get(LColor_color col) const
{
	lcolor_map::const_iterator cit = colormap.find(col);
	if (cit != colormap.end())
		return cit->second;

	if (lcolor.getX11Name(col) == "grey40")
		colormap[col] = Qt::lightGray;
	else if (lcolor.getX11Name(col) == "grey60")
		colormap[col] = Qt::gray;
	else if (lcolor.getX11Name(col) == "grey80")
		colormap[col] = Qt::darkGray;
	else if (lcolor.getX11Name(col) == "grey90")
		colormap[col] = Qt::darkGray;
	else
		colormap[col] = QColor(lcolor.getX11Name(col).c_str());

	return colormap[col];
}


void LColorCache::clear()
{
	colormap.clear();
}
