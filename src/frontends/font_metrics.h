// -*- C++ -*-
/**
 * \file font_metrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FONT_METRICS_H
#define FONT_METRICS_H

#include "support/types.h"

#include <string>


class LyXFont;

/**
 * A namespace holding helper functions for determining
 * the screen dimensions of fonts.
 *
 * The geometry is the standard typographical geometry,
 * as follows :
 *
 * --------------+------------------<maxAscent
 *               |          |
 *               <-------> (right bearing)
 *               <-> (left bearing)
 * char ascent>___          |
 *               ^   oooo   |  oooo
 *   origin>____ |  oo  oo  | oo  oo
 *              \|  oo  oo  | oo  oo
 * --------------+---ooooo--|--oooo-<baseline
 *               |      oo  |
 * char          |  oo  oo  |
 * descent>______|   oooo   |
 *               <-  width ->
 * --------------+----------+-------<maxDescent
 *
 */
namespace font_metrics {
	/// return the maximum ascent of the font
	int maxAscent(LyXFont const & f);
	/// return the maximum descent of the font
	int maxDescent(LyXFont const & f);
	/// return the maximum descent of the font
	inline int maxHeight(LyXFont const & f) {
		return maxAscent(f) + maxDescent(f);
	}
	/// return the ascent of the char in the font
	int ascent(lyx::char_type c, LyXFont const & f);
	/// return the descent of the char in the font
	int descent(lyx::char_type c, LyXFont const & f);
	/// return the descent of the char in the font
	inline int height(lyx::char_type c, LyXFont const & f)
	{
		return ascent(c, f) + descent(c, f);
	}
	/// return the left bearing of the char in the font
	int lbearing(lyx::char_type c, LyXFont const & f);
	/// return the right bearing of the char in the font
	int rbearing(lyx::char_type c, LyXFont const & f);
	/// return the inner width of the char in the font
	inline int center(lyx::char_type c, LyXFont const & f) {
		return (rbearing(c, f) - lbearing(c, f)) / 2;
	}
	/// return the width of the string in the font
	int width(lyx::char_type const * s, size_t n, LyXFont const & f);
	/// return the width of the char in the font
	inline int width(lyx::char_type c, LyXFont const & f)
	{
		lyx::char_type tmp[2] = { c, L'\0'};
		return width(tmp, 1, f);
	}
	/// return the width of the string in the font
	inline int width(lyx::docstring const & s, LyXFont const & f)
	{
	    return s.empty() ? 0 : width(s.data(), s.length(), f);
	}
	/// FIXME ??
	int signedWidth(lyx::docstring const & s, LyXFont const & f);
	/**
	 * fill in width,ascent,descent with the values for the
	 * given string in the font.
	 */
	void rectText(lyx::docstring const & str, LyXFont const & font,
		int & width,
		int & ascent,
		int & descent);
	/**
	 * fill in width,ascent,descent with the values for the
	 * given string in the font for a button.
	 */
	void buttonText(lyx::docstring const & str, LyXFont const & font,
		int & width,
		int & ascent,
		int & descent);
}

#endif // FONT_METRICS_H
