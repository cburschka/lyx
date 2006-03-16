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

const QColor grey40(0x66, 0x66, 0x66);
const QColor grey60(0x99, 0x99, 0x99);
const QColor grey80(0xcc, 0xcc, 0xcc);
const QColor grey90(0xe5, 0xe5, 0xe5);


LColorCache::LColorCache()
{
}

QColor const & LColorCache::get(LColor_color col) const
{
	lcolor_map::const_iterator cit = colormap.find(col);
	if (cit != colormap.end())
		return cit->second;

	if (lcolor.getX11Name(col) == "grey40")
		colormap[col] = grey40;
	else if (lcolor.getX11Name(col) == "grey60")
		colormap[col] = grey60;
	else if (lcolor.getX11Name(col) == "grey80")
		colormap[col] = grey80;
	else if (lcolor.getX11Name(col) == "grey90")
		colormap[col] = grey90;
	else
		colormap[col] = QColor(lcolor.getX11Name(col).c_str());

	return colormap[col];
}


void LColorCache::clear()
{
	colormap.clear();
}
