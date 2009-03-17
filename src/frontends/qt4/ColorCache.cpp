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
#include "ColorSet.h"

namespace lyx {

void ColorCache::init()
{
	for (int col = 0; col <= Color_ignore; ++col)
		lcolors_[col] = QColor(lcolor.getX11Name(ColorCode(col)).c_str());
	initialized_ = true;
}


/// get the given color
QColor ColorCache::get(Color color) const
{
	if (!initialized_)
		const_cast<ColorCache *>(this)->init();
	if (color <= Color_ignore && color.mergeColor == Color_ignore)
		return lcolors_[color.baseColor];
	if (color.mergeColor != Color_ignore) {
		// FIXME: This would ideally be done in the Color class, but
		// that means that we'd have to use the Qt code in the core.
		QColor base_color = get(color.baseColor).toRgb();
		QColor merge_color = get(color.mergeColor).toRgb();
		return QColor(
			(base_color.red() + merge_color.red()) / 2,
			(base_color.green() + merge_color.green()) / 2,
			(base_color.blue() + merge_color.blue()) / 2);
	}
	// used by branches
	return QColor(lcolor.getX11Name(color.baseColor).c_str()); 
}


QColor const rgb2qcolor(RGBColor const & rgb)
{
	return QColor(rgb.r, rgb.g, rgb.b);
}


} // namespace lyx
