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

void ColorCache::init()
{
	for (int col = 0; col <= Color_ignore; ++col)
		lcolors_[col] = QColor(lcolor.getX11Name(ColorCode(col)).c_str());
	initialized_ = true;
}


/// get the given color
QColor ColorCache::get(ColorCode color) const
{
	if (!initialized_)
		const_cast<ColorCache *>(this)->init();
	if (color <= Color_ignore)
		return lcolors_[color];
	// used by branches
	return QColor(lcolor.getX11Name(color).c_str()); 
}


QColor const rgb2qcolor(RGBColor const & rgb)
{
	return QColor(rgb.r, rgb.g, rgb.b);
}


} // namespace lyx
