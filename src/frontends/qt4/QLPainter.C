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

#include "QWorkArea.h"
#include "QLImage.h"

#include "ColorCache.h"
#include "FontLoader.h"

#include "debug.h"
#include "language.h"
#include "LColor.h"

#include "frontends/font_metrics.h"

#include <QPainter>
#include <QPicture>
#include <QPixmap>
#include <QImage>

using std::endl;
using std::string;


QLPainter::~QLPainter()
{
}

QLPainter::QLPainter(QWorkArea * qwa)
	: Painter(), qwa_(qwa)
{
}


int QLPainter::paperWidth() const
{
	return qwa_->viewport()->width();
}


int QLPainter::paperHeight() const
{
	return qwa_->viewport()->height();
}

void QLPainter::setQPainterPen(QPainter & qp, LColor_color col,
	Painter::line_style ls, Painter::line_width lw)
{
	if (col == current_color_ && ls == current_ls_ && lw == current_lw_)
		return;

	current_color_ = col;
	current_ls_ = ls;
	current_lw_ = lw;

	QPen pen = qp.pen();

	pen.setColor(lcolorcache.get(col));

	switch (ls) {
		case line_solid: pen.setStyle(Qt::SolidLine); break;
		case line_onoffdash: pen.setStyle(Qt::DotLine); break;
	}

	switch (lw) {
		case line_thin: pen.setWidth(0); break;
		case line_thick: pen.setWidth(3); break;
	}

	qp.setPen(pen);
}

void QLPainter::point(int x, int y, LColor_color col)
{
	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, col);
	qp.drawPoint(x, y);
}


void QLPainter::line(int x1, int y1, int x2, int y2,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, col, ls, lw);
	qp.drawLine(x1, y1, x2, y2);
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

	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, col, ls, lw);
	qp.drawPolyline(points.get(), np);
}


void QLPainter::rectangle(int x, int y, int w, int h,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, col, ls, lw);
	qp.drawRect(x, y, w, h);
}


void QLPainter::fillRectangle(int x, int y, int w, int h, LColor_color col)
{
	QPainter qp(qwa_->paintDevice());
	qp.fillRect(x, y, w, h, lcolorcache.get(col));
}


void QLPainter::fillPolygon(int const * xp, int const * yp,
	int np, LColor_color col)
{
	// Must use new as np is not known at compile time.
	boost::scoped_array<QPoint> points(new QPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].setX(xp[i]);
		points[i].setY(yp[i]);
	}

	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, col);
	qp.setBrush(lcolorcache.get(col));
	qp.drawPolygon(points.get(), np);
	qp.setBrush(Qt::NoBrush);
}


void QLPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, LColor_color col)
{
	// LyX usings 1/64ths degree, Qt usings 1/16th
	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, col);
	qp.drawArc(x, y, w, h, a1 / 4, a2 / 4);
}


void QLPainter::image(int x, int y, int w, int h,
	lyx::graphics::Image const & i)
{
	lyx::graphics::QLImage const & qlimage =
		static_cast<lyx::graphics::QLImage const &>(i);

	fillRectangle(x, y, w, h, LColor::graphicsbg);

	QPainter qp(qwa_->paintDevice());
	qp.drawImage(x, y, qlimage.qimage(), 0, 0, w, h);
}


void QLPainter::text(int x, int y, string const & s, LyXFont const & f)
{
	return text(x, y, s.data(), s.length(), f);
}


void QLPainter::text(int x, int y, char c, LyXFont const & f)
{
	char s[2] = { c, '\0' };
	return text(x, y, s, 1, f);
}


void QLPainter::smallCapsText(int x, int y,
	QString const & s, LyXFont const & f)
{
	LyXFont smallfont(f);
	smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);

	QFont const & qfont = fontloader.get(f);
	QFont const & qsmallfont = fontloader.get(smallfont);
	QFontMetrics const & qfontm = QFontMetrics(qfont);
	QFontMetrics const & qsmallfontm = QFontMetrics(qsmallfont);

	QPainter qp(qwa_->paintDevice());
	setQPainterPen(qp, f.realColor());
	int tmpx = x;
	size_t ls = s.length();
	for (size_t i = 0; i < ls; ++i) {
		QChar const c = s[i].upper();
		if (c != s.at(i)) {
			qp.setFont(qsmallfont);
			qp.drawText(tmpx, y, c);
			tmpx += qsmallfontm.width(c);
		} else {
			qp.setFont(qfont);
			qp.drawText(tmpx, y, c);
			tmpx += qfontm.width(c);
		}
	}
}


void QLPainter::text(int x, int y, char const * s, size_t ls,
	LyXFont const & f)
{
	Encoding const * encoding = f.language()->encoding();
	if (f.isSymbolFont())
		encoding = encodings.symbol_encoding();

	QString str;
	str.setLength(ls);
	for (int i = 0; i < ls; ++i)
		str[i] = QChar(encoding->ucs(s[i]));

	// HACK: QT3 refuses to show single compose characters
	//       Still needed with Qt4?
	if (ls == 1 && str[0].unicode() >= 0x05b0 && str[0].unicode() <= 0x05c2)
		str = ' ' + str;

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		QPainter qp(qwa_->paintDevice());
		setQPainterPen(qp, f.realColor());
		qp.setFont(fontloader.get(f));
		// We need to draw the text as LTR as we use our own bidi code.
		qp.setLayoutDirection(Qt::LeftToRight);
		qp.drawText(x, y, str, -1);
	} else {
		smallCapsText(x, y, str, f);
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, font_metrics::width(s, ls, f));
	}

}
/// draw a pixmap from the image cache
void QLPainter::drawPixmap(int x, int y, QPixmap const & pixmap)
{
	QPainter qp(qwa_->paintDevice());
	qp.drawPixmap(x, y, pixmap);
}

void QLPainter::drawImage(int x, int y, QImage const & image)
{
	QPainter qp(qwa_->paintDevice());
	qp.drawImage(x, y, image);
}
