// -*- C++ -*-
/**
 * \file lcolorcache.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LCOLORCACHE_H
#define LCOLORCACHE_H

#include <map>

#include <qcolor.h>


class LColor_color;


// FIXME: use a fixed-size array not a map ?

/**
 * Cache from LColor to QColor.
 */
class LColorCache {
public:
	LColorCache();

	/// get the given color
	QColor const & get(LColor_color color) const;

	/// clear all colors
	void clear();

private:
	typedef std::map<LColor_color, QColor> lcolor_map;

	mutable lcolor_map colormap;
};

/// singleton instance
extern LColorCache lcolorcache;

#endif // LCOLORCACHE_H
