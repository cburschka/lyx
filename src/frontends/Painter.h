// -*- C++ -*-
/**
 * \file Painter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef PAINTER_H
#define PAINTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "LColor.h"

class LyXFont;

namespace grfx {
	class Image;
}

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
 * queue (perhaps generated via Screen::expose), does the copy onto
 * the actual WorkArea widget take place. Paints are wrapped in (possibly
 * recursive) calls to start() and end() to facilitate the backing pixmap
 * management.
 *
 * Note that the methods return *this for convenience.
 */
class Painter {
public:
	/// possible line widths
	enum line_width {
		line_thin, //< thin line
		line_thick //< thick line
	};

	/// possible line styles
	enum line_style {
		line_solid, //< solid line
		line_onoffdash //< dashes with spaces
	};

	virtual ~Painter() {}

	/// begin painting
	virtual void start() {}
 
	/// end painting
	virtual void end() {}
 
	/// return the width of the work area in pixels
	virtual int paperWidth() const = 0;
	/// return the height of the work area in pixels
	virtual int paperHeight() const = 0;

	/// draw a line from point to point
	virtual Painter & line(
		int x1, int y1,
		int x2, int y2,
		LColor::color = LColor::foreground,
		line_style = line_solid,
		line_width = line_thin) = 0;

	/**
	 * lines -  draw a set of lines
	 * @param xp array of points' x co-ords
	 * @param yp array of points' y co-ords
	 * @param np size of the points array
	 */
	virtual Painter & lines(
		int const * xp,
		int const * yp,
		int np,
		LColor::color = LColor::foreground,
		line_style = line_solid,
		line_width = line_thin) = 0;

	/// draw a rectangle
	virtual Painter & rectangle(
		int x, int y,
		int w, int h,
		LColor::color = LColor::foreground,
		line_style = line_solid,
		line_width = line_thin) = 0;
	
	/// draw a filled rectangle
	virtual Painter & fillRectangle(
		int x, int y,
		int w, int h,
		LColor::color) = 0;
	
	/// draw a filled (irregular) polygon
	virtual Painter & fillPolygon(
		int const * xp,
		int const * yp,
		int np,
		LColor::color = LColor::foreground) = 0;

	/// draw an arc
	virtual Painter & arc(
		int x, int y,
		unsigned int w, unsigned int h,
		int a1, int a2,
		LColor::color = LColor::foreground) = 0;

	/// draw a pixel
	virtual Painter & point(
		int x, int y,
		LColor::color = LColor::foreground) = 0;
	
	/// draw a filled rectangle with the shape of a 3D button
	virtual Painter & button(int x, int y,
		int w, int h);

	/// draw an image from the image cache
	virtual Painter & image(int x, int y,
		int w, int h,
		grfx::Image const & image) = 0;
	
	/// draw a string at position x, y (y is the baseline)
	virtual Painter & text(int x, int y,
		string const & str, LyXFont const & f) = 0;

	/**
	 * Draw a string at position x, y (y is the baseline)
	 * This is just for fast drawing
	 */
	virtual Painter & text(int x, int y,
		char const * str, size_t l,
		LyXFont const & f) = 0;

	/// draw a char at position x, y (y is the baseline)
	virtual Painter & text(int x, int y,
		char c, LyXFont const & f) = 0;

	/**
	 * Draw a string and enclose it inside a rectangle. If
	 * back color is specified, the background is cleared with
	 * the given color. If frame is specified, a thin frame is drawn
	 * around the text with the given color.
	 */
	Painter & rectText(int x, int baseline,
		string const & string,
		LyXFont const & font,
		LColor::color back = LColor::none,
		LColor::color frame = LColor::none);

	/// draw a string and enclose it inside a button frame
	Painter & buttonText(int x,
		int baseline, string const & s,
		LyXFont const & font);

protected:
	/// check the font, and if set, draw an underline
	void underline(LyXFont const & f, 
		int x, int y, int width);
	
	/// draw a bevelled button border
        Painter & buttonFrame(int x, int y, int w, int h);
};

#endif // PAINTER_H
