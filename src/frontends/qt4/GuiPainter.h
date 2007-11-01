// -*- C++ -*-
/**
 * \file GuiPainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPAINTER_H
#define GUIPAINTER_H

#include "frontends/Painter.h"

#include <QPainter>
#include <stack>

class QString;

namespace lyx {

class FontInfo;

namespace frontend {

/**
 * GuiPainter - a painter implementation for Qt4
 */
class GuiPainter : public QPainter, public Painter {
public:
	GuiPainter(QPaintDevice *);
	virtual ~GuiPainter();

	/// draw a line from point to point
	virtual void line(
		int x1, int y1,
		int x2, int y2,
		ColorCode,
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
		ColorCode,
		line_style = line_solid,
		line_width = line_thin);

	/// draw a rectangle
	virtual void rectangle(
		int x, int y,
		int w, int h,
		ColorCode,
		line_style = line_solid,
		line_width = line_thin);

	/// draw a filled rectangle
	virtual void fillRectangle(
		int x, int y,
		int w, int h,
		ColorCode);

	/// draw an arc
	virtual void arc(
		int x, int y,
		unsigned int w, unsigned int h,
		int a1, int a2,
		ColorCode);

	/// draw a pixel
	virtual void point(
		int x, int y,
		ColorCode);

	/// draw an image from the image cache
	virtual void image(int x, int y,
		int w, int h,
		lyx::graphics::Image const & image);

	/// draw a string at position x, y (y is the baseline)
	virtual int text(int x, int y,
		docstring const & str, FontInfo const & f);

	/// draw a char at position x, y (y is the baseline)
	virtual int text(int x, int y, char_type c, FontInfo const & f);

	/// start monochrome painting mode, i.e. map every color into [min,max]
	virtual void enterMonochromeMode(ColorCode const & min, 
		ColorCode const & max);
	/// leave monochrome painting mode
	virtual void leaveMonochromeMode();
	
private:
	/// draw small caps text
	/**
	\return width of the drawn text.
	*/
	int smallCapsText(int x, int y,
		QString const & str, FontInfo const & f);

	/// set pen parameters
	void setQPainterPen(QColor const & col,
		line_style ls = line_solid,
		line_width lw = line_thin);

	QColor current_color_;
	Painter::line_style current_ls_;
	Painter::line_width current_lw_;
	///
	std::stack<QColor> monochrome_min_;
	///
	std::stack<QColor> monochrome_max_;
	/// convert into Qt color, possibly applying the monochrome mode
	QColor computeColor(ColorCode col);
	/// possibly apply monochrome mode
	QColor filterColor(QColor const & col);
	///
	QString generateStringSignature(QString const & str, FontInfo const & f);	
};

} // namespace frontend
} // namespace lyx

#endif // GUIPAINTER_H
