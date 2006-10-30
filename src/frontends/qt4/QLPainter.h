// -*- C++ -*-
/**
 * \file QLPainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLPAINTER_H
#define QLPAINTER_H

#include "Painter.h"

#include <QPainter>

class QString;

namespace lyx {

class LyXFont;

namespace frontend {

class GuiWorkArea;

/**
 * QLPainter - a painter implementation for Qt4
 */
class QLPainter : public QPainter, public Painter {
public:
	/// draw a line from point to point
	virtual void line(
		int x1, int y1,
		int x2, int y2,
		LColor_color,
		line_style = line_solid,
		line_width = line_thin);

	/**
	 * lines -  draw a set of lines
	 * @param xp array of points' x co-ords
	 * @param yp array of points' y co-ords
	 * @param np size of the points array
	 */
	virtual void lines(
		int const * xp,
		int const * yp,
		int np,
		LColor_color,
		line_style = line_solid,
		line_width = line_thin);

	/// draw a rectangle
	virtual void rectangle(
		int x, int y,
		int w, int h,
		LColor_color,
		line_style = line_solid,
		line_width = line_thin);

	/// draw a filled rectangle
	virtual void fillRectangle(
		int x, int y,
		int w, int h,
		LColor_color);

	/// draw an arc
	virtual void arc(
		int x, int y,
		unsigned int w, unsigned int h,
		int a1, int a2,
		LColor_color);

	/// draw a pixel
	virtual void point(
		int x, int y,
		LColor_color);

	/// draw an image from the image cache
	virtual void image(int x, int y,
		int w, int h,
		lyx::graphics::Image const & image);

	/// draw a string at position x, y (y is the baseline)
	virtual int text(int x, int y,
		lyx::docstring const & str, LyXFont const & f);

	/** Draw a string at position x, y (y is the baseline)
	 *  This is just for fast drawing
	 */
	virtual int text(int x, int y,
                lyx::char_type const * str, size_t l,
		LyXFont const & f);

	/// draw a char at position x, y (y is the baseline)
	virtual int text(int x, int y,
                lyx::char_type c, LyXFont const & f);

private:
	friend class GuiWorkArea;
	QLPainter(QWidget *);
	virtual ~QLPainter();

	/// draw small caps text
	/**
	\return width of the drawn text.
	*/
	int smallCapsText(int x, int y,
		QString const & str, LyXFont const & f);

	/// set pen parameters
	void setQPainterPen(LColor_color col,
		line_style ls = line_solid,
		line_width lw = line_thin);

	/// the working area
	QWidget * qwa_;

	LColor::color current_color_;
	Painter::line_style current_ls_;
	Painter::line_width current_lw_;
};

} // namespace frontend
} // namespace lyx

#endif // QLPAINTER_H
