// -*- C++ -*-
/**
 * \file FontMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FONT_METRICS_H
#define FONT_METRICS_H

#include "support/strfwd.h"

/**
 * A class holding helper functions for determining
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
 * Caution: All char_type and docstring arguments of any method of this class
 * are no UCS4 chars or strings if the font is a symbol font. They simply
 * denote the code points of the font instead. You have to keep this in mind
 * when you implement the methods in a frontend. You must not pass these
 * parameters to a unicode conversion function in particular.
 */

namespace lyx {

class Dimension;

namespace frontend {

class FontMetrics
{
public:
	virtual ~FontMetrics() {}

	/// return the maximum ascent of the font
	virtual int maxAscent() const = 0;
	/// return the maximum descent of the font
	virtual int maxDescent() const = 0;
	/// return default dimension of the font.
	/// \warning \c width is set to zero.
	virtual Dimension const defaultDimension() const = 0;
	/// return the width of the char in the font
	virtual int width(char_type c) const = 0;
	/// return the ascent of the char in the font
	virtual int ascent(char_type c) const = 0;
	/// return the descent of the char in the font
	virtual int descent(char_type c) const = 0;
	/// return the left bearing of the char in the font
	virtual int lbearing(char_type c) const = 0;
	/// return the right bearing of the char in the font
	virtual int rbearing(char_type c) const = 0;
	/// return the width of the string in the font
	virtual int width(docstring const & s) const = 0;
	/// FIXME ??
	virtual int signedWidth(docstring const & s) const = 0;
	/**
	 * return the x offset of a position in the string. The
	 * direction of the string is forced, and the returned value
	 * is from the left edge of the word, not from the start of the string.
	 */
	virtual int pos2x(docstring const & s, int pos, bool rtl) const = 0;
	/**
	 * return the position in the string for a given x offset. The
	 * direction of the string is forced, and the returned value
	 * is from the left edge of the word, not from the start of the string.
	 * the offset x is updated to match the closest position in the string.
	 */
	virtual int x2pos(docstring const & s, int & x, bool rtl) const = 0;
	/// return char dimension for the font.
	virtual Dimension const dimension(char_type c) const = 0;
	/**
	 * fill in width,ascent,descent with the values for the
	 * given string in the font.
	 */
	virtual void rectText(docstring const & str,
		int & width,
		int & ascent,
		int & descent) const = 0;
	/**
	 * fill in width,ascent,descent with the values for the
	 * given string in the font for a button.
	 */
	virtual void buttonText(docstring const & str,
		int & width,
		int & ascent,
		int & descent) const = 0;

	/// return the maximum descent of the font
	inline int maxHeight() const { return maxAscent() + maxDescent(); }

	/// return the descent of the char in the font
	inline int height(char_type c) const { return ascent(c) + descent(c); }

	/// return the inner width of the char in the font
	inline int center(char_type c) const {
		return (rbearing(c) - lbearing(c)) / 2;
	}
};


} // namespace frontend

class Font;
class FontInfo;

/// Implementation is in Application.cpp
frontend::FontMetrics const & theFontMetrics(Font const & f);
frontend::FontMetrics const & theFontMetrics(FontInfo const & fi);

} // namespace lyx

#endif // FONT_METRICS_H
