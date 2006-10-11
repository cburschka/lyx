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

using lyx::char_type;
using lyx::docstring;

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

QLPainter::~QLPainter()
{
}


QLPainter::QLPainter(GuiWorkArea * qwa)
	: Painter(), qwa_(qwa)
{
}


void QLPainter::start()
{
	qp_.reset(new QPainter(qwa_->paintDevice()));
	// new QPainter has default QPen:
	current_color_ = LColor::black;
	current_ls_ = line_solid;
	current_lw_ = line_thin;
}


void QLPainter::end()
{
	qp_->end();
}


int QLPainter::paperWidth() const
{
	return qwa_->viewport()->width();
}


int QLPainter::paperHeight() const
{
	return qwa_->viewport()->height();
}

void QLPainter::setQPainterPen(LColor_color col,
	Painter::line_style ls, Painter::line_width lw)
{
	if (col == current_color_ && ls == current_ls_ && lw == current_lw_)
		return;

	current_color_ = col;
	current_ls_ = ls;
	current_lw_ = lw;

	QPen pen = qp_.get()->pen();

	pen.setColor(guiApp->colorCache().get(col));

	switch (ls) {
		case line_solid: pen.setStyle(Qt::SolidLine); break;
		case line_onoffdash: pen.setStyle(Qt::DotLine); break;
	}

	switch (lw) {
		case line_thin: pen.setWidth(0); break;
		case line_thick: pen.setWidth(3); break;
	}

	qp_.get()->setPen(pen);
}


void QLPainter::point(int x, int y, LColor_color col)
{
	setQPainterPen(col);
	qp_->drawPoint(x, y);
}


void QLPainter::line(int x1, int y1, int x2, int y2,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setQPainterPen(col, ls, lw);
	qp_->drawLine(x1, y1, x2, y2);
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
	qp_->drawPolyline(points.get(), np);
}


void QLPainter::rectangle(int x, int y, int w, int h,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setQPainterPen(col, ls, lw);
	qp_->drawRect(x, y, w, h);
}


void QLPainter::fillRectangle(int x, int y, int w, int h, LColor_color col)
{
	qp_->fillRect(x, y, w, h, guiApp->colorCache().get(col));
}


void QLPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, LColor_color col)
{
	// LyX usings 1/64ths degree, Qt usings 1/16th
	setQPainterPen(col);
	qp_->drawArc(x, y, w, h, a1 / 4, a2 / 4);
}


void QLPainter::image(int x, int y, int w, int h,
	lyx::graphics::Image const & i)
{
	lyx::graphics::QLImage const & qlimage =
		static_cast<lyx::graphics::QLImage const &>(i);

	fillRectangle(x, y, w, h, LColor::graphicsbg);

	qp_->drawImage(x, y, qlimage.qimage(), 0, 0, w, h);
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
	size_t ls = s.length();
	for (unsigned int i = 0; i < ls; ++i) {
		QChar const c = s[i].toUpper();
		if (c != s.at(i)) {
			qp_->setFont(qsmallfont);
		} else {
			qp_->setFont(qfont);
		}
		qp_->drawText(x + textwidth, y, c);
		textwidth += qp_->fontMetrics().width(c);
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
		qp_->setFont(fi.font);
		// We need to draw the text as LTR as we use our own bidi code.
		qp_->setLayoutDirection(Qt::LeftToRight);
		qp_->drawText(x, y, str);
		textwidth = qp_->fontMetrics().width(str);
	} else {
		textwidth = smallCapsText(x, y, str, f);
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, textwidth);
	}

	return textwidth;
}


void QLPainter::drawPixmap(int x, int y, QPixmap const & pixmap)
{
	qp_->drawPixmap(x, y, pixmap);
}


void QLPainter::drawImage(int x, int y, QImage const & image)
{
	qp_->drawImage(x, y, image);
}

} // namespace frontend
} // namespace lyx
