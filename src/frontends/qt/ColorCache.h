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

#include "Color.h"

#include <QColor>
#include <QPalette>

namespace lyx {

struct RGBColor;

/**
 * Cache from Color to QColor.
 */
class ColorCache
{
public:
	///
	ColorCache() : initialized_(false) {}

	/// get the given color (depends on LyXRC::use_system_color)
	QColor get(Color const & color) const;

	/// get the given color
	QColor get(Color const & color, bool use_system_colors) const;

	/// is this color replaced when LyXRC::use_system_color is true?
	bool isSystem(ColorCode color) const;

	/// change the undelying palette
	void setPalette(QPalette const pal) { pal_ = pal; clear(); }

	/// clear all colors
	void clear() { initialized_ = false; }

private:
	///
	void init();
	///
	QColor lcolors_[Color_ignore + 1];
	///
	bool initialized_;
	///
	QPalette pal_;
};

///
QColor const rgb2qcolor(RGBColor const &);

} // namespace lyx

#endif // COLORCACHE_H
