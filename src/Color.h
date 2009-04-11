// -*- C++ -*-
/**
 * \file Color.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COLOR_H
#define COLOR_H

#include "ColorCode.h"

#include "support/strfwd.h"

namespace lyx {

/**
 * \class Color
 *
 * A class holding a definition of a certain color.
 *
 * A color can be one of the following kinds:
 *
 * - a single color, then mergeColor = Color_ignore
 * - a merged color, i.e. the average of the base and merge colors.
 */

class Color
{
public:
	///
	Color(ColorCode base_color = Color_none);
	
	/// comparison operators.
	//@{
	bool operator==(Color const & color) const;
	bool operator!=(Color const & color) const;
	bool operator<(Color const & color) const;
	bool operator<=(Color const & color) const;
	//@}

	/// the base color
	ColorCode baseColor;
	/// The color that is merged with the base color. Set
	/// mergeColor to Color_ignore if no merging is wanted.
	ColorCode mergeColor;
};

std::ostream & operator<<(std::ostream & os, Color color);

std::string const X11hexname(RGBColor const & col);
RGBColor rgbFromHexName(std::string const & x11hexname);
std::string const outputLaTeXColor(RGBColor const & color);

} // namespace lyx

#endif // COLOR_H
