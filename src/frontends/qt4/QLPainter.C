/**
 * \file QLPainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLPainter.h"

#include "GuiApplication.h"
#include "GuiWorkArea.h"
#include "QLImage.h"

#include "GuiApplication.h"
#include "qt_helpers.h"

#include "debug.h"
#include "language.h"
#include "LColor.h"

#include "frontends/FontMetrics.h"

#include "support/unicode.h"

#include <QPainter>
#include <QPicture>
#include <QPixmap>
#include <QImage>


using std::endl;
using std::string;

namespace lyx {
namespace frontend {

QLPainter::QLPainter(QWidget * qwa)
	: qwa_(qwa)
{
	//lyxerr << "QLPainter::start()" << endl;
	QPainter::begin(qwa_);
	setRenderHint(QPainter::TextAntialiasing);
	// new QPainter has default QPen:
	current_color_ = LColor::black;
	current_ls_ = line_solid;
	current_lw_ = line_thin;
}


QLPainter::~QLPainter()
{
	QPainter::end();
	//lyxerr << "QLPainter::end()" << endl;
}


int QLPainter::paperWidth() const
{
	return qwa_->width();
}


int QLPainter::paperHeight() const
{
	return qwa_->height();
}


void QLPainter::setQPainterPen(LColor_color col,
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


void QLPainter::point(int x, int y, LColor_color col)
{
	setQPainterPen(col);
	drawPoint(x, y);
}


void QLPainter::line(int x1, int y1, int x2, int y2,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setQPainterPen(col, ls, lw);
	drawLine(x1, y1, x2, y2);
}


void QLPainter::lines(int const * xp, int const * yp, int np,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	// FIXME ?

	// Must use new as np is not known at compile time.
	boost::scoped_array<QPoint> points(new QPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].setX(xp[i]);
		points[i].setY(yp[i]);
	}

	setQPainterPen(col, ls, lw);
	drawPolyline(points.get(), np);
}


void QLPainter::rectangle(int x, int y, int w, int h,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setQPainterPen(col, ls, lw);
	drawRect(x, y, w, h);
}


void QLPainter::fillRectangle(int x, int y, int w, int h, LColor_color col)
{
	fillRect(x, y, w, h, guiApp->colorCache().get(col));
}


void QLPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, LColor_color col)
{
	// LyX usings 1/64ths degree, Qt usings 1/16th
	setQPainterPen(col);
	drawArc(x, y, w, h, a1 / 4, a2 / 4);
}


void QLPainter::image(int x, int y, int w, int h, graphics::Image const & i)
{
	graphics::QLImage const & qlimage =
		static_cast<graphics::QLImage const &>(i);

	fillRectangle(x, y, w, h, LColor::graphicsbg);

	drawImage(x, y, qlimage.qimage(), 0, 0, w, h);
}


int QLPainter::text(int x, int y, docstring const & s, LyXFont const & f)
{
	 return text(x, y, reinterpret_cast<char_type const *>(s.data()), s.length(), f);
}


int QLPainter::text(int x, int y, char_type c, LyXFont const & f)
{
	char_type s[2] = { c, char_type('\0') };
	return text(x, y, s, 1, f);
}


int QLPainter::smallCapsText(int x, int y,
	QString const & s, LyXFont const & f)
{
	LyXFont smallfont(f);
	smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);

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
		drawText(x + textwidth, y, c);
		textwidth += fontMetrics().width(c);
	}
	return textwidth;
}


int QLPainter::text(int x, int y, char_type const * s, size_t ls,
	LyXFont const & f)
{
#if 0
	Encoding const * encoding = f.language()->encoding();
	if (f.isSymbolFont())
		encoding = encodings.symbol_encoding();
#endif

	QString str;
	ucs4_to_qstring(s, ls, str);

#if 0
	// HACK: QT3 refuses to show single compose characters
	//       Still needed with Qt4?
	if (ls == 1 && str[0].unicode() >= 0x05b0 && str[0].unicode() <= 0x05c2)
		str = ' ' + str;
#endif

	QLFontInfo & fi = guiApp->guiFontLoader().fontinfo(f);

	int textwidth;

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		setQPainterPen(f.realColor());
		if (font() != fi.font)
			setFont(fi.font);
		// We need to draw the text as LTR as we use our own bidi code.
		setLayoutDirection(Qt::LeftToRight);
		drawText(x, y, str);
		// Here we use the font width cache instead of
		//   textwidth = fontMetrics().width(str);
		// because the above is awfully expensive on MacOSX
		textwidth = guiApp->guiFontLoader().metrics(f).width(s, ls);
	} else {
		textwidth = smallCapsText(x, y, str, f);
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, textwidth);
	}

	return textwidth;
}


} // namespace frontend
} // namespace lyx
