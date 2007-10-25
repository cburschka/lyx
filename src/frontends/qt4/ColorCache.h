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

#include <map>

namespace lyx {

struct RGBColor;


// FIXME: use a fixed-size array not a map ?

/**
 * Cache from Color to QColor.
 */
class ColorCache {
public:
	ColorCache() {}

	/// get the given color
	QColor const & get(ColorCode color) const;

	/// clear all colors
	void clear();

private:
	typedef std::map<ColorCode, QColor> lcolor_map;

	mutable lcolor_map colormap;
};

///
QColor const rgb2qcolor(RGBColor const &);

} // namespace lyx

#endif // COLORCACHE_H
