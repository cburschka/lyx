/**
 * \file XPainter.C
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "XPainter.h"
#include "LString.h"
#include "debug.h"
#include "XWorkArea.h"
#include "xfont_metrics.h"
#include "ColorHandler.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"

#ifdef USE_XFORMS_IMAGE_LOADER
#include "xformsImage.h"
#else
#include "graphics/GraphicsImageXPM.h"
#endif

#include "support/LAssert.h"
#include "support/lstrings.h"

#include <boost/scoped_array.hpp>

#include <cmath>

using std::endl;
using std::max;

 
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

 
Painter & XPainter::point(int x, int y, LColor::color c)
{
	XDrawPoint(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(c), x, y);
	return *this;
}


Painter & XPainter::line(int x1, int y1, 
	int x2, int y2,
	LColor::color col,
	line_style ls,
	line_width lw)
{
	XDrawLine(fl_get_display(), owner_.getPixmap(), 
		lyxColorHandler->getGCLinepars(ls, lw, col),
		x1, y1, x2, y2);
	return *this;
}


Painter & XPainter::lines(int const * xp, int const * yp, 
	int np,
	LColor::color col,
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

	return *this;
}      


Painter & XPainter::rectangle(int x, int y, 
	int w, int h,
	LColor::color col,
	line_style ls,
	line_width lw)
{
	XDrawRectangle(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCLinepars(ls, lw, col), 
		x, y, w, h);
	return *this;
}


Painter & XPainter::fillRectangle(int x, int y, 
	int w, int h,
	LColor::color col)
{
	XFillRectangle(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(col), x, y, w, h);
	return *this;
}


Painter & XPainter::fillPolygon(int const * xp, int const * yp, 
	int np, LColor::color col)
{
	boost::scoped_array<XPoint> points(new XPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}
 
	XFillPolygon(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(col), points.get(), 
		np, Nonconvex, CoordModeOrigin);
 
	return *this;
}

 
Painter & XPainter::arc(int x, int y,
	unsigned int w, unsigned int h,
	int a1, int a2, LColor::color col)
{
        XDrawArc(fl_get_display(), owner_.getPixmap(),
		lyxColorHandler->getGCForeground(col),
		x, y, w, h, a1, a2);
        return *this;
}

 
Painter & XPainter::image(int x, int y, 
	int w, int h,
	grfx::Image const & i)
{
#ifdef USE_XFORMS_IMAGE_LOADER
	grfx::xformsImage const & image = static_cast<grfx::xformsImage const &>(i);
#else
	grfx::ImageXPM const & image = static_cast<grfx::ImageXPM const &>(i);
#endif
	
	XGCValues val;
	val.function = GXcopy;
	GC gc = XCreateGC(fl_get_display(), owner_.getPixmap(),
		GCFunction, &val);
	XCopyArea(fl_get_display(), image.getPixmap(), owner_.getPixmap(), 
		gc, 0, 0, w, h, x, y);
	XFreeGC(fl_get_display(), gc);
	return *this;
}


Painter & XPainter::text(int x, int y, 
	string const & s, LyXFont const & f)
{
	return text(x, y, s.data(), s.length(), f);
}


Painter & XPainter::text(int x, int y, 
	char c, LyXFont const & f)
{
	char s[2] = { c, '\0' };
	return text(x, y, s, 1, f);
}


Painter & XPainter::text(int x, int y, 
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
		return *this;
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
	
	return *this;
}


Painter & XPainter::text(int x, int y, 
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
	
	return *this;
}
