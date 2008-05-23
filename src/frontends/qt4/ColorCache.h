// -*- C++ -*-
/**
 * \file ColorCache.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COLORCACHE_H
#define COLORCACHE_H

#include "ColorCode.h"

#include <QColor>

namespace lyx {

struct RGBColor;

/**
 * Cache from Color to QColor.
 */
class ColorCache
{
public:
	ColorCache() { init(); }

	/// get the given color
	QColor const & get(ColorCode color) const { return lcolors_[color]; }

	/// clear all colors
	void clear() { init(); }

private:
	///
	void init();
	///
	QColor lcolors_[Color_ignore + 1];
};

///
QColor const rgb2qcolor(RGBColor const &);

} // namespace lyx

#endif // COLORCACHE_H
