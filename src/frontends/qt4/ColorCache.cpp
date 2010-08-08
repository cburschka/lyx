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

#include "LyXRC.h"

#include "ColorCache.h"
#include "ColorSet.h"

namespace lyx {

void ColorCache::setColor(int col, QPalette::ColorRole cr)
{
	lcolors_[col] = pal_.brush(QPalette::Active, cr).color();
}


void ColorCache::init()
{
	if (lyxrc.use_system_colors) {
		for (int col = 0; col <= Color_ignore; ++col) {
			switch (ColorCode(col)) {
			case Color_background:
			case Color_commentbg:
			case Color_greyedoutbg:
			case Color_mathbg:
			case Color_graphicsbg:
			case Color_mathmacrobg:
			case Color_mathcorners:
				setColor(col, QPalette::Base);
				break;
				
			case Color_foreground:
			case Color_cursor:
			case Color_preview:
			case Color_tabularline:
			case Color_previewframe:
				setColor(col, QPalette::Text);
				break;
				
			case Color_selection:
				setColor(col, QPalette::Highlight);
				break;
			case Color_selectiontext:
				setColor(col, QPalette::HighlightedText);
				break;
			default:
				lcolors_[col] = QColor(lcolor.getX11Name(ColorCode(col)).c_str());
			}
		}
	} else {
		for (int col = 0; col <= Color_ignore; ++col) 
			lcolors_[col] = QColor(lcolor.getX11Name(ColorCode(col)).c_str());
	}
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
