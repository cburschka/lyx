// -*- C++ -*-
/**
 * \file Painter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PAINTER_H
#define PAINTER_H

#include "support/strfwd.h"
#include "support/types.h"

namespace lyx {

class Font;
class FontInfo;

namespace graphics { class Image; }

namespace frontend {

/**
 * Painter - A painter class to encapsulate all graphics parameters and operations
 *
 * Every graphics operation in LyX should be made by this class. The
 * painter is used for drawing on the WorkArea, and is passed around
 * during draw operations.
 *
 * It hides low level windows system parameters so insets and other
 * clients don't have to worry about them and we can control graphics and
 * GUI toolkit dependent drawing functions inside this single class.
 *
 * The intention for a toolkit is that it uses these methods to paint
 * onto a backing pixmap. Only when expose events arrive via the event
 * queue, does the copy onto the actual WorkArea widget take place.
 *
 * Caution: All char_type and docstring arguments of the text drawing
 * methods of this class are no UCS4 chars or strings if the font is a
 * symbol font. They simply denote the code points of the font instead.
 * You have to keep this in mind when you implement the methods in a
 * frontend. You must not pass these parameters to a unicode conversion
 * function in particular.
 */
class Painter {
public:
	Painter(double pixel_ratio) : pixel_ratio_(pixel_ratio) {}

	static const int thin_line;

	/// possible line styles
	enum line_style {
		line_solid, //< solid line
		line_solid_aliased, //< solid line, no anti-aliasing (used as a
		                    // workaround to painting issues)
		line_onoffdash //< dashes with spaces
	};

	/// possible fill styles
	enum fill_style {
		fill_none,
		fill_oddeven,
		fill_winding
	};

	/// possible character styles of preedit string.
	/// This is used for CJK input method support.
	enum preedit_style {
		preedit_default, //< when unselecting, no cursor and dashed underline.
		preedit_selecting, //< when selecting.
		preedit_cursor //< with cursor.
	};

	virtual ~Painter() {}

	/// draw a line from point to point
	virtual void line(int x1, int y1, int x2, int y2, Color,
		line_style = line_solid, int line_width = thin_line) = 0;

	/**
	 * lines -  draw a set of lines
	 * @param xp array of points' x co-ords
	 * @param yp array of points' y co-ords
	 * @param np size of the points array
	 */
	virtual void lines(int const * xp, int const * yp, int np, Color,
		fill_style = fill_none, line_style = line_solid,
		int line_width = thin_line) = 0;

	/**
	 * path -  draw a path with bezier curves
	 * @param xp array of points' x co-ords
	 * @param yp array of points' y co-ords
	 * @param c1x array of first control points' x co-ords
	 * @param c1y array of first control points' y co-ords
	 * @param c2x array of second control points' x co-ords
	 * @param c2y array of second control points' y co-ords
	 * @param np size of the points array
	 */
	virtual void path(int const * xp, int const * yp,
		int const * c1x, int const * c1y,
		int const * c2x, int const * c2y,
		int np, Color,
		fill_style = fill_none, line_style = line_solid,
		int line_width = thin_line) = 0;

	/// draw a rectangle
	virtual void rectangle(int x, int y, int w, int h, Color,
		line_style = line_solid, int line_width = thin_line) = 0;

	/// draw a filled rectangle
	virtual void fillRectangle(int x, int y, int w, int h, Color) = 0;

	/// draw an arc
	virtual void arc(int x, int y, unsigned int w, unsigned int h,
		int a1, int a2, Color) = 0;

	/// draw a pixel
	virtual void point(int x, int y, Color) = 0;

	/// draw an image from the image cache
	virtual void image(int x, int y, int w, int h,
		graphics::Image const & image) = 0;

	/// draw a string at position x, y (y is the baseline).
	virtual void text(int x, int y, docstring const & str, FontInfo const & f) = 0;

	/// draw a char at position x, y (y is the baseline)
	virtual void text(int x, int y, char_type c, FontInfo const & f) = 0;

	/** draw a string at position x, y (y is the baseline). The
	 * text direction is enforced by the \c Font.
	 */
	virtual void text(int x, int y, docstring const & str, Font const & f,
                      double wordspacing, double textwidth) = 0;

	/** draw a string at position x, y (y is the baseline), but
	 * make sure that the part between \c from and \c to is in
	 * \c other color. The text direction is enforced by the \c Font.
	 */
	virtual void text(int x, int y, docstring const & str, Font const & f,
	                  Color other, size_type from, size_type to,
                      double wordspacing, double textwidth) = 0;

	// Returns true if the painter does not actually paint.
	virtual bool isNull() const = 0;

	double pixelRatio() const { return pixel_ratio_; }

	/// draw the underbar, strikeout, xout, uuline and uwave font attributes
	virtual void textDecoration(FontInfo const & f, int x, int y, int width) = 0;

	/**
	 * Draw a string and enclose it inside a rectangle. If
	 * back color is specified, the background is cleared with
	 * the given color. If frame is specified, a thin frame is drawn
	 * around the text with the given color.
	 */
	virtual void rectText(int x, int baseline, docstring const & str,
		FontInfo const & font, Color back, Color frame) = 0;

	/// draw a string and enclose it inside a button frame
	virtual void buttonText(int x, int baseline, docstring const & s,
		FontInfo const & font, Color back, Color frame, int offset) = 0;

	/// draw a character of a preedit string for cjk support.
	virtual int preeditText(int x, int y,
		char_type c, FontInfo const & f, preedit_style style) = 0;

	/// start monochrome painting mode, i.e. map every color into [min,max]
	virtual void enterMonochromeMode(Color const & min,
		Color const & max) = 0;
	/// leave monochrome painting mode
	virtual void leaveMonochromeMode() = 0;
	/// draws a wavy line that can be used for underlining.
	virtual void wavyHorizontalLine(int x, int y, int width, ColorCode col) = 0;
private:
	/// Ratio between physical pixels and device-independent pixels
	double pixel_ratio_;
};

} // namespace frontend
} // namespace lyx

#endif // PAINTER_H
