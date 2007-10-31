/**
 * \file ColorCache.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ColorCache.h"

#include "Color.h"

#include <string>

namespace lyx {

const QColor grey40(0x66, 0x66, 0x66);
const QColor grey60(0x99, 0x99, 0x99);
const QColor grey80(0xcc, 0xcc, 0xcc);
const QColor grey90(0xe5, 0xe5, 0xe5);
const QColor none = Qt::black;

QColor const & ColorCache::get(ColorCode col) const
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
	else if (lcolor.getX11Name(col) == "none")
		colormap[col] = none;
	else
		colormap[col] = QColor(lcolor.getX11Name(col).c_str());

	return colormap[col];
}


void ColorCache::clear()
{
	colormap.clear();
}


QColor const rgb2qcolor(RGBColor const & rgb)
{
	return QColor(rgb.r, rgb.g, rgb.b);
}


} // namespace lyx
