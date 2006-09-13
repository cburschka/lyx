/**
 * \file QLPainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLPainter.h"

#include "QWorkArea.h"
#include "QLImage.h"
#include "lcolorcache.h"
#include "qfont_loader.h"

#include "debug.h"
#include "language.h"
#include "LColor.h"

#include "support/unicode.h"

#include "frontends/font_metrics.h"

#include <qpainter.h>

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

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


QPainter & QLPainter::setPen(LColor_color c,
	Painter::line_style ls, Painter::line_width lw)
{
	QPen pen = qp_->pen();

	pen.setColor(lcolorcache.get(c));

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


void QLPainter::point(int x, int y, LColor_color c)
{
	setPen(c).drawPoint(x, y);
}


void QLPainter::line(int x1, int y1, int x2, int y2,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setPen(col, ls, lw).drawLine(x1, y1, x2, y2);
}


void QLPainter::lines(int const * xp, int const * yp, int np,
	LColor_color col,
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
}


void QLPainter::rectangle(int x, int y, int w, int h,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	setPen(col, ls, lw).drawRect(x, y, w, h);
}


void QLPainter::fillRectangle(int x, int y, int w, int h, LColor_color col)
{
	qp_->fillRect(x, y, w, h, lcolorcache.get(col));
}


void QLPainter::arc(int x, int y, unsigned int w, unsigned int h,
	int a1, int a2, LColor_color col)
{
	// LyX usings 1/64ths degree, Qt usings 1/16th
	setPen(col).drawArc(x, y, w, h, a1 / 4, a2 / 4);
}


void QLPainter::image(int x, int y, int w, int h,
	lyx::graphics::Image const & i)
{
	lyx::graphics::QLImage const & qlimage =
		static_cast<lyx::graphics::QLImage const &>(i);

	fillRectangle(x, y, w, h, LColor::graphicsbg);
	bitBlt(qp_->device(), x, y, &qlimage.qpixmap(), 0, 0, w, h);
}


void QLPainter::text(int x, int y, docstring const & s, LyXFont const & f)
{
	lyxerr << "Drawing string" << endl;
	return text(x, y, reinterpret_cast<lyx::char_type const *>(s.data()), s.length(), f);
}


void QLPainter::text(int x, int y, lyx::char_type c, LyXFont const & f)
{
	char_type s[2] = { c, L'\0' };
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
		// Brain-dead MSVC wants at(i) rather than operator[]
		QChar const c = s.at(i).upper();
		if (c != s.at(i)) {
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


void QLPainter::text(int x, int y, lyx::char_type const * s, size_t ls,
	LyXFont const & f)
{
	lyxerr << "Drawing lyx::char_type const * s" << endl;
	setPen(f.realColor());

#if 0
	Encoding const * encoding = f.language()->encoding();
	if (f.isSymbolFont())
		encoding = encodings.symbol_encoding();
#endif


#if 0
	QString str;
	str.setLength(ls);
	for (size_t i = 0; i < ls; ++i)
		// Brain-dead MSVC wants at(i) rather than operator[]
		str.at(i) = QChar(encoding->ucs(s[i]));
#else
	//std::vector<lyx::char_type> in(s, s + ls);
	//std::vector<unsigned short> ucs2 = ucs4_to_ucs2(in);
	std::vector<unsigned short> ucs2 = ucs4_to_ucs2(s, ls);
	ucs2.push_back(0);
	QString str = QString::fromUcs2(&ucs2[0]);
#endif

#if 0
	// HACK: QT3 refuses to show single compose characters
	if (ls == 1 && str[0].unicode() >= 0x05b0 && str[0].unicode() <= 0x05c2)
		str = ' ' + str;
#endif

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		qp_->setFont(fontloader.get(f));
		// We need to draw the text as LTR as we use our own bidi
		// code.
		qp_->drawText(x, y, str, -1, QPainter::LTR);
	} else {
		smallCapsText(x, y, str, f);
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, font_metrics::width(s, ls, f));
	}
}

} // namespace frontend
} // namespace lyx
