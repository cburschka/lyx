// -*- C++ -*-
/**
 * \file QLPainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QLPAINTER_H
#define QLPAINTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Painter.h"
#include "LString.h"
#include "LColor.h"

#include <boost/scoped_ptr.hpp>

class LyXFont;
class QWorkArea;
class QPainter;

/**
 * QLPainter - a painter implementation for Xlib
 */
class QLPainter : public Painter {
public:
	QLPainter(QWorkArea &);

	/// begin painting
	virtual void start();

	/// end painting
	virtual void end();

	/// return the width of the work area in pixels
	virtual int paperWidth() const;
	/// return the height of the work area in pixels
	virtual int paperHeight() const;

	/// draw a line from point to point
	virtual Painter & line(
		int x1, int y1,
		int x2, int y2,
		LColor::color = LColor::foreground,
		line_style = line_solid,
		line_width = line_thin);

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
		line_width = line_thin);

	/// draw a rectangle
	virtual Painter & rectangle(
		int x, int y,
		int w, int h,
		LColor::color = LColor::foreground,
		line_style = line_solid,
		line_width = line_thin);

	/// draw a filled rectangle
	virtual Painter & fillRectangle(
		int x, int y,
		int w, int h,
		LColor::color);

	/// draw a filled (irregular) polygon
	virtual Painter & fillPolygon(
		int const * xp,
		int const * yp,
		int np,
		LColor::color = LColor::foreground);

	/// draw an arc
	virtual Painter & arc(
		int x, int y,
		unsigned int w, unsigned int h,
		int a1, int a2,
		LColor::color = LColor::foreground);

	/// draw a pixel
	virtual Painter & point(
		int x, int y,
		LColor::color = LColor::foreground);

	/// draw an image from the image cache
	virtual Painter & image(int x, int y,
		int w, int h,
		grfx::Image const & image);

	/// draw a string at position x, y (y is the baseline)
	virtual Painter & text(int x, int y,
		string const & str, LyXFont const & f);

	/** Draw a string at position x, y (y is the baseline)
	 *  This is just for fast drawing
	 */
	virtual Painter & text(int x, int y,
		char const * str, size_t l,
		LyXFont const & f);

	/// draw a char at position x, y (y is the baseline)
	virtual Painter & text(int x, int y,
		char c, LyXFont const & f);

private:
	/// draw small caps text
	void smallCapsText(int x, int y,
		char const * str, size_t l,
		LyXFont const & f);

	/// set pen parameters
	QPainter & setPen(LColor::color c,
		line_style ls = line_solid,
		line_width lw = line_thin);

	/// our owner who we paint upon
	QWorkArea & owner_;

	/// our qt painter
	boost::scoped_ptr<QPainter> qp_;

	/// recursion check
	int paint_check_;
};

#endif // QLPAINTER_H
