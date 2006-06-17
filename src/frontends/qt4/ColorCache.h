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

#include "LColor.h"

#include <map>

#include <QColor>


namespace lyx {
struct RGBColor;
}


// FIXME: use a fixed-size array not a map ?

/**
 * Cache from LColor to QColor.
 */
class ColorCache {
public:
	ColorCache();

	/// get the given color
	QColor const & get(LColor_color color) const;

	/// clear all colors
	void clear();

private:
	typedef std::map<LColor_color, QColor> lcolor_map;

	mutable lcolor_map colormap;
};

/// singleton instance
extern ColorCache lcolorcache;

///
QColor const rgb2qcolor(lyx::RGBColor const &);
#endif // COLORCACHE_H
