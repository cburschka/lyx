/**
 * \file QLPainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "font_metrics.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "debug.h"
#include "LyXView.h"
#include "encoding.h"
#include "language.h"

#include "QWorkArea.h"
#include "qfont_loader.h"
#include "QLPainter.h"
#include "QLImage.h"

#include <boost/scoped_array.hpp>

#include <qpainter.h>
#include <qbrush.h>
#include <qcolor.h>

#include <iostream>

using std::endl;


QLPainter::QLPainter(QWorkArea & qwa)
	: Painter(), owner_(qwa), paint_check_(0)
{
	qp_.reset(new QPainter);
}


void QLPainter::start()
{
	if (++paint_check_ == 1)
		qp_->begin(owner_.getPixmap());
}


void QLPainter::end()
{
	if (paint_check_ == 0) {
		lyxerr << "ended painting whilst not painting ??" << endl;
	} else if (--paint_check_ == 0) {
		qp_->end();
	}
}


int QLPainter::paperWidth() const
{
	return owner_.workWidth();
}


int QLPainter::paperHeight() const
{
	return owner_.workHeight();
}


QPainter & QLPainter::setPen(LColor::color c,
	Painter::line_style ls, Painter::line_width lw)
{
	QPen pen = qp_->pen();

	pen.setColor(lcolor.getX11Name(c).c_str());

	switch (ls) {
		case line_solid: pen.setStyle(QPen::SolidLine); break;
		case line_onoffdash: pen.setStyle(QPen::DotLine); break;
	}

	switch (lw) {
		case line_thin: pen.setWidth(0); break;
		case line_thick: pen.setWidth(3); break;
	}

	qp_->setPen(pen);
	return *qp_;
}


Painter & QLPainter::point(int x, int y, LColor::color c)
{
	setPen(c).drawPoint(x, y);
	return *this;
}


Painter & QLPainter::line(int x1, int y1,
	int x2, int y2,
	LColor::color col,
	line_style ls,
	line_width lw)
{
	setPen(col, ls, lw).drawLine(x1, y1, x2, y2);
	return *this;
}


Painter & QLPainter::lines(int const * xp, int const * yp,
	int np,
	LColor::color col,
	line_style ls,
	line_width lw)
{
	// FIXME ?

	// Must use new as np is not known at compile time.
	boost::scoped_array<QCOORD> points(new QCOORD[np * 2]);

	for (int i = 0, j = 0; i < np; ++i) {
		points[j++] = xp[i];
		points[j++] = yp[i];
	}

	setPen(col, ls, lw).drawPolyline(QPointArray(np, points.get()));

	return *this;
}


Painter & QLPainter::rectangle(int x, int y,
	int w, int h,
	LColor::color col,
	line_style ls,
	line_width lw)
{
	//lyxerr << "rectangle " << x<<","<<y << " " <<w<<","<<h<<endl;
	setPen(col, ls, lw).drawRect(x, y, w, h);
	return *this;
}


Painter & QLPainter::fillRectangle(int x, int y,
	int w, int h,
	LColor::color col)
{
	//lyxerr << "fillRectangle " << x<<","<<y << " " <<w<<","<<h<<endl;
	qp_->fillRect(x, y, w, h, QColor(lcolor.getX11Name(col).c_str()));
	return *this;
}


Painter & QLPainter::fillPolygon(int const * xp, int const * yp,
	int np, LColor::color col)
{
	// Must use new as np is not known at compile time.
	boost::scoped_array<QCOORD> points(new QCOORD[np * 2]);

	//if (1) return *this;

	for (int i = 0, j = 0; i < np; ++i) {
		points[j++] = xp[i];
		points[j++] = yp[i];
	}

	setPen(col);
	qp_->setBrush(lcolor.getX11Name(col).c_str());
	qp_->drawPolygon(QPointArray(np, points.get()));
	qp_->setBrush(Qt::NoBrush);

	return *this;
}


Painter & QLPainter::arc(int x, int y,
	unsigned int w, unsigned int h,
	int a1, int a2, LColor::color col)
{
	lyxerr[Debug::GUI] << "arc: " << x<<","<<y
		<< " " << w<<","<<h << ", angles "
		<< a1 << " - " << a2 << endl;
	// LyX usings 1/64ths degree, Qt usings 1/16th
	setPen(col).drawArc(x, y, w, h, a1 / 4, a2 / 4);
	return *this;
}


Painter & QLPainter::image(int x, int y,
	int w, int h,
	grfx::Image const & i)
{
	qp_->drawPixmap(x, y, static_cast<grfx::QLImage const &>(i).qpixmap(), 0, 0, w, h);
	return *this;
}


Painter & QLPainter::text(int x, int y,
	string const & s, LyXFont const & f)
{
	return text(x, y, s.data(), s.length(), f);
}


Painter & QLPainter::text(int x, int y,
	char c, LyXFont const & f)
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

	int tmpx = x;
	size_t ls = s.length();
	for (size_t i = 0; i < ls; ++i) {
		QChar const c = s[i].upper();
		if (c != s[i]) {
			qp_->setFont(qsmallfont);
			qp_->drawText(tmpx, y, c);
			tmpx += qsmallfontm.width(c);
		} else {
			qp_->setFont(qfont);
			qp_->drawText(tmpx, y, c);
			tmpx += qfontm.width(c);
		}
	}
}


Painter & QLPainter::text(int x, int y,
	char const * s, size_t ls,
	LyXFont const & f)
{
	setPen(f.color());

	Encoding const * encoding = f.language()->encoding();
	if (f.isSymbolFont())
		encoding = encodings.symbol_encoding();

	QString str;
#if QT_VERSION >= 0x030000
	str.setLength(ls);
	for (size_t i = 0; i < ls; ++i)
		str[i] = QChar(encoding->ucs(s[i]));
	// HACK: QT3 refuses to show single compose characters
	if (ls = 1 && str[0].unicode() >= 0x05b0 && str[0].unicode() <= 0x05c2)
		str = ' ' + str;
#else
	for (size_t i = 0; i < ls; ++i)
		str += QChar(encoding->ucs(s[i]));
#endif

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		qp_->setFont(fontloader.get(f));
#if QT_VERSION >= 0x030000
		// We need to draw the text as LTR as we use our own bidi
		// code.
		qp_->drawText(x, y, str, -1, QPainter::LTR);
#else
		qp_->drawText(x, y, str);
#endif
	} else {
		smallCapsText(x, y, str, f);
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, font_metrics::width(s, ls, f));
	}

	return *this;
}
