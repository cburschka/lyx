/**
 * \file XPainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "XPainter.h"

#include "ColorHandler.h"
#include "xfont_metrics.h"
#include "xformsImage.h"
#include "XWorkArea.h"

#include "font_metrics.h"

#include "encoding.h"
#include "language.h"
#include "LColor.h"
#include "lyxfont.h"
#include "lyxrc.h"

#include "support/lstrings.h"

using lyx::support::uppercase;

using std::string;


XPainter::XPainter(XWorkArea & xwa)
	: Painter(), owner_(xwa)
{
}


int XPainter::paperWidth() const
{
	return owner_.workWidth();
}


int XPainter::paperHeight() const
{
	return owner_.workHeight();
}


void XPainter::point(int x, int y, LColor_color c)
{
	XDrawPoint(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(c), x, y);
}


void XPainter::line(int x1, int y1,
	int x2, int y2,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	XDrawLine(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCLinepars(ls, lw, col),
		x1, y1, x2, y2);
}


void XPainter::lines(int const * xp, int const * yp,
	int np,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	boost::scoped_array<XPoint> points(new XPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

	XDrawLines(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCLinepars(ls, lw, col),
		points.get(), np, CoordModeOrigin);
}


void XPainter::rectangle(int x, int y,
	int w, int h,
	LColor_color col,
	line_style ls,
	line_width lw)
{
	XDrawRectangle(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCLinepars(ls, lw, col),
		x, y, w, h);
}


void XPainter::fillRectangle(int x, int y,
	int w, int h,
	LColor_color col)
{
	XFillRectangle(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(col), x, y, w, h);
}


void XPainter::fillPolygon(int const * xp, int const * yp,
	int np, LColor_color col)
{
	boost::scoped_array<XPoint> points(new XPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

	XFillPolygon(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(col), points.get(),
		np, Nonconvex, CoordModeOrigin);
}


void XPainter::arc(int x, int y,
	unsigned int w, unsigned int h,
	int a1, int a2, LColor_color col)
{
	XDrawArc(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(col),
		x, y, w, h, a1, a2);
}


void XPainter::image(int x, int y,
			  int w, int h,
			  lyx::graphics::Image const & i)
{
	lyx::graphics::xformsImage const & image =
		static_cast<lyx::graphics::xformsImage const &>(i);

	XGCValues val;
	val.function = GXcopy;
	GC gc = XCreateGC(fl_get_display(), owner_.getPixmap(),
		GCFunction, &val);
	XCopyArea(fl_get_display(), image.getPixmap(), owner_.getPixmap(),
		gc, 0, 0, w, h, x, y);
	XFreeGC(fl_get_display(), gc);
}


void XPainter::text(int x, int y,
	string const & s, LyXFont const & f)
{
	return text(x, y, s.data(), s.length(), f);
}


void XPainter::text(int x, int y,
	char c, LyXFont const & f)
{
	char s[2] = { c, '\0' };
	return text(x, y, s, 1, f);
}


void XPainter::text(int x, int y,
	char const * s, size_t ls,
	LyXFont const & f)
{
	if (lyxrc.font_norm_type == LyXRC::ISO_10646_1) {
		boost::scoped_array<XChar2b> xs(new XChar2b[ls]);
		Encoding const * encoding = f.language()->encoding();
		LyXFont font(f);
		if (f.isSymbolFont()) {
#ifdef USE_UNICODE_FOR_SYMBOLS
			font.setFamily(LyXFont::ROMAN_FAMILY);
			font.setShape(LyXFont::UP_SHAPE);
#endif
			encoding = encodings.symbol_encoding();
		}
		for (size_t i = 0; i < ls; ++i) {
			Uchar c = encoding->ucs(s[i]);
			xs[i].byte1 = c >> 8;
			xs[i].byte2 = c & 0xff;
		}
		text(x, y, xs.get(), ls, font);
		return;
	}

	GC gc = lyxColorHandler->getGCForeground(f.realColor());
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		xfont_metrics::XSetFont(fl_get_display(), gc, f);
		XDrawString(fl_get_display(), owner_.getPixmap(), gc, x, y, s, ls);
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		int tmpx = x;
		for (size_t i = 0; i < ls; ++i) {
			char const c = uppercase(s[i]);
			if (c != s[i]) {
				xfont_metrics::XSetFont(fl_get_display(), gc, smallfont);
				XDrawString(fl_get_display(), owner_.getPixmap(), gc,
					tmpx, y, &c, 1);
				tmpx += xfont_metrics::XTextWidth(smallfont, &c, 1);
			} else {
				xfont_metrics::XSetFont(fl_get_display(), gc, f);
				XDrawString(fl_get_display(), owner_.getPixmap(), gc,
					tmpx, y, &c, 1);
				tmpx += xfont_metrics::XTextWidth(f, &c, 1);
			}
		}
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, font_metrics::width(s, ls, f));
	}
}


void XPainter::text(int x, int y,
	XChar2b const * s, size_t ls,
	LyXFont const & f)
{
	GC gc = lyxColorHandler->getGCForeground(f.realColor());
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		xfont_metrics::XSetFont(fl_get_display(), gc, f);
		XDrawString16(fl_get_display(), owner_.getPixmap(), gc, x, y, s, ls);
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		static XChar2b c;
		int tmpx = x;
		for (size_t i = 0; i < ls; ++i) {
			if (s[i].byte1)
				c = s[i];
			else {
				c.byte1 = s[i].byte1;
				c.byte2 = uppercase(s[i].byte2);
			}
			if (c.byte2 != s[i].byte2) {
				xfont_metrics::XSetFont(fl_get_display(), gc, smallfont);
				XDrawString16(fl_get_display(), owner_.getPixmap(), gc,
					tmpx, y, &c, 1);
				tmpx += xfont_metrics::XTextWidth16(smallfont, &c, 1);
			} else {
				xfont_metrics::XSetFont(fl_get_display(), gc, f);
				XDrawString16(fl_get_display(), owner_.getPixmap(), gc,
					tmpx, y, &c, 1);
				tmpx += xfont_metrics::XTextWidth16(f, &c, 1);
			}
		}
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, xfont_metrics::width(s, ls, f));
	}
}
