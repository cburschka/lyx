// -*- C++ -*-
/**
 * \file GPainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GPAINTER_H
#define GPAINTER_H

#include <config.h>
#include <X11/Xft/Xft.h>

#include <map>
#include "frontends/Painter.h"
#include "LString.h"

class LyXFont;
class GWorkArea;

/**
 * GPainter - a painter implementation for Gtkmm
 */
class GPainter : public Painter {
public:
	GPainter(GWorkArea &);

	/// return the width of the work area in pixels
	virtual int paperWidth() const;
	/// return the height of the work area in pixels
	virtual int paperHeight() const;

	void setForeground(Glib::RefPtr<Gdk::GC> gc, LColor::color clr);
	void setLineParam(Glib::RefPtr<Gdk::GC> gc,
			  line_style ls, line_width lw);
	XftColor * getXftColor(LColor::color clr);
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
		lyx::graphics::Image const & image);

	/// draw a string at position x, y (y is the baseline)
	virtual Painter & text(int x, int y,
		string const & str, LyXFont const & f);

	/** Draw a string at position x, y (y is the baseline)
	 *  This is just for fast drawing
	 */
	virtual Painter & text(int x, int y,
		char const * str, size_t l,
		LyXFont const & f);

        virtual Painter & text(int x, int y, wchar_t const * str, int l,
			       LyXFont const & f);

	/// draw a char at position x, y (y is the baseline)
	virtual Painter & text(int x, int y,
			       char c, LyXFont const & f);

	/// draw a wide string at position x, y
	Painter & text(int x, int y,
		XChar2b const * str, size_t l,
		LyXFont const & f);

private:
	/// our owner who we paint upon
	GWorkArea & owner_;
};

#endif // XPAINTER_H
