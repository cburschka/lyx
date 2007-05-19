/**
 * \file QLPainter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <QTextLayout>

#include "QLPainter.h"

#include "GuiApplication.h"
#include "GuiFontMetrics.h"
#include "QLImage.h"

#include "GuiApplication.h"
#include "qt_helpers.h"

#include "debug.h"
#include "Language.h"
#include "Color.h"

#include "support/unicode.h"

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

QLPainter::QLPainter(QPaintDevice * device)
	: QPainter(device), Painter()
{
	// new QPainter has default QPen:
	current_color_ = Color::black;
	current_ls_ = line_solid;
	current_lw_ = line_thin;
}


QLPainter::~QLPainter()
{
	QPainter::end();
	//lyxerr << "QLPainter::end()" << endl;
}


void QLPainter::setQPainterPen(Color_color col,
	Painter::line_style ls, Painter::line_width lw)
{
	if (col == current_color_ && ls == current_ls_ && lw == current_lw_)
		return;

	current_color_ = col;
	current_ls_ = ls;
	current_lw_ = lw;

	QPen pen = QPainter::pen();

	pen.setColor(guiApp->colorCache().get(col));

	switch (ls) {
		case line_solid: pen.setStyle(Qt::SolidLine); break;
		case line_onoffdash: pen.setStyle(Qt::DotLine); break;
	}

	switch (lw) {
		case line_thin: pen.setWidth(0); break;
		case line_thick: pen.setWidth(3); break;
	}

	setPen(pen);
}


void QLPainter::point(int x, int y, Color_color col)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(col);
	drawPoint(x, y);
}


void QLPainter::line(int x1, int y1, int x2, int y2,
	Color_color col,
	line_style ls,
	line_width lw)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(col, ls, lw);
	bool const do_antialiasing = renderHints() & TextAntialiasing
		&& x1 != x2 && y1 != y2;
	setRenderHint(Antialiasing, do_antialiasing);
	drawLine(x1, y1, x2, y2);
	setRenderHint(Antialiasing, false);
}


void QLPainter::lines(int const * xp, int const * yp, int np,
	Color_color col,
	line_style ls,
	line_width lw)
{
	if (!isDrawingEnabled())
		return;
	
	// Must use new as np is not known at compile time.
	boost::scoped_array<QPoint> points(new QPoint[np]);
	
	bool antialias = false;
	for (int i = 0; i < np; ++i) {
		points[i].setX(xp[i]);
		points[i].setY(yp[i]);
		if (i != 0) 
			antialias |= xp[i-1] != xp[i] && yp[i-1] != yp[i];
	}
 	setQPainterPen(col, ls, lw);
	bool const text_is_antialiased = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, antialias && text_is_antialiased);
	drawPolyline(points.get(), np);
	setRenderHint(Antialiasing, false);
}


void QLPainter::rectangle(int x, int y, int w, int h,
	Color_color col,
	line_style ls,
	line_width lw)
{
	if (!isDrawingEnabled())
		return;

	setQPainterPen(col, ls, lw);
	drawRect(x, y, w, h);
}


void QLPainter::fillRectangle(int x, int y, int w, int h, Color_color col)
{
	fillRect(x, y, w, h, guiApp->colorCache().get(col));
}


void QLPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, Color_color col)
{
	if (!isDrawingEnabled())
		return;

	// LyX usings 1/64ths degree, Qt usings 1/16th
	setQPainterPen(col);
	bool const do_antialiasing = renderHints() & TextAntialiasing;
	setRenderHint(Antialiasing, do_antialiasing);
	drawArc(x, y, w, h, a1 / 4, a2 / 4);
	setRenderHint(Antialiasing, false);
}


void QLPainter::image(int x, int y, int w, int h, graphics::Image const & i)
{
	graphics::QLImage const & qlimage =
		static_cast<graphics::QLImage const &>(i);

	fillRectangle(x, y, w, h, Color::graphicsbg);

	if (!isDrawingEnabled())
		return;

	drawImage(x, y, qlimage.qimage(), 0, 0, w, h);
}


int QLPainter::text(int x, int y, char_type c, Font const & f)
{
	docstring s(1, c);
	return text(x, y, s, f);
}


int QLPainter::smallCapsText(int x, int y,
	QString const & s, Font const & f)
{
	Font smallfont(f);
	smallfont.decSize().decSize().setShape(Font::UP_SHAPE);

	QFont const & qfont = guiApp->guiFontLoader().get(f);
	QFont const & qsmallfont = guiApp->guiFontLoader().get(smallfont);

	setQPainterPen(f.realColor());
	int textwidth = 0;
	size_t const ls = s.length();
	for (unsigned int i = 0; i < ls; ++i) {
		QChar const c = s[i].toUpper();
		if (c != s.at(i)) {
			setFont(qsmallfont);
		} else {
			setFont(qfont);
		}
		if (isDrawingEnabled())
			drawText(x + textwidth, y, c);
		textwidth += fontMetrics().width(c);
	}
	return textwidth;
}


int QLPainter::text(int x, int y, docstring const & s,
		Font const & f)
{
	/* Caution: The following ucs4 to QString conversions work for symbol fonts
	only because they are no real conversions but simple casts in reality.
	When we want to draw a symbol or calculate the metrics we pass the position
	of the symbol in the font (as given in lib/symbols) as a char_type to the
	frontend. This is just wrong, because the symbol is no UCS4 character at
	all. You can think of this number as the code point of the symbol in a
	custom symbol encoding. It works because this char_type is lateron again
	interpreted as a position in the font again.
	The correct solution would be to have extra functions for symbols, but that
	would require to duplicate a lot of frontend and mathed support code.
	*/
	QString str = toqstr(s);

#if 0
	// HACK: QT3 refuses to show single compose characters
	//       Still needed with Qt4?
	if (ls == 1 && str[0].unicode() >= 0x05b0 && str[0].unicode() <= 0x05c2)
		str = ' ' + str;
#endif

	QLFontInfo & fi = guiApp->guiFontLoader().fontinfo(f);

	int textwidth;

	if (f.realShape() != Font::SMALLCAPS_SHAPE) {
		setQPainterPen(f.realColor());
		if (font() != fi.font)
			setFont(fi.font);
		// We need to draw the text as LTR as we use our own bidi code.
		setLayoutDirection(Qt::LeftToRight);
		if (isDrawingEnabled()) {
			LYXERR(Debug::PAINTING) << "draw " << std::string(str.toUtf8())
				<< " at " << x << "," << y << std::endl;
			// Qt4 does not display a glyph whose codepoint is the
			// same as that of a soft-hyphen (0x00ad), unless it
			// occurs at a line-break. As a kludge, we force Qt to
			// render this glyph using a one-column line.
			if (s.size() == 1 && str[0].unicode() == 0x00ad) {
				QTextLayout adsymbol(str);
				adsymbol.setFont(fi.font);
				adsymbol.beginLayout();
				QTextLine line = adsymbol.createLine();
				line.setNumColumns(1);
				line.setPosition(QPointF(0, -line.ascent()));
				adsymbol.endLayout();
				line.draw(this, QPointF(x, y));
			} else
				drawText(x, y, str);
		}
		// Here we use the font width cache instead of
		//   textwidth = fontMetrics().width(str);
		// because the above is awfully expensive on MacOSX
		textwidth = fi.metrics->width(str);
	} else {
		textwidth = smallCapsText(x, y, str, f);
	}

	if (f.underbar() == Font::ON) {
		underline(f, x, y, textwidth);
	}

	return textwidth;
}


} // namespace frontend
} // namespace lyx

