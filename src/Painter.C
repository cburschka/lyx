/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2001 The LyX Team
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Painter.h"
#include "LString.h"
#include "debug.h"
#include "lyxfont.h"
#include "WorkArea.h"
#include "font.h"
#include "ColorHandler.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"

#include "frontends/GUIRunTime.h"
#include "frontends/support/LyXImage.h"

#include "support/LAssert.h"
#include "support/lstrings.h"

#include <boost/smart_ptr.hpp>

#include <cmath>

using std::endl;
using std::max;

Painter::Painter(WorkArea & wa)
	: PainterBase(wa)
{
	display = GUIRunTime::x11Display();
}


// Basic drawing routines

PainterBase & Painter::point(int x, int y, LColor::color c)
{
	XDrawPoint(display, owner.getPixmap(),
		   lyxColorHandler->getGCForeground(c), x, y);
	return *this;
}


PainterBase & Painter::line(int x1, int y1, int x2, int y2,
			LColor::color col,
			enum line_style ls,
			enum line_width lw)
{
	XDrawLine(display, owner.getPixmap(), 
		  lyxColorHandler->getGCLinepars(ls, lw, col),
		  x1, y1, x2, y2);
	return *this;
}


PainterBase & Painter::lines(int const * xp, int const * yp, int np,
			     LColor::color col,
			     enum line_style ls,
			     enum line_width lw)
{
	boost::scoped_array<XPoint> points(new XPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

        XDrawLines(display, owner.getPixmap(),
		   lyxColorHandler->getGCLinepars(ls, lw, col), 
		   points.get(), np, CoordModeOrigin);

	return *this;
}      


PainterBase & Painter::rectangle(int x, int y, int w, int h,
				 LColor::color col,
				 enum line_style ls,
				 enum line_width lw)
{
	XDrawRectangle(display, owner.getPixmap(),
		       lyxColorHandler->getGCLinepars(ls, lw, col), 
		       x, y, w, h);
	return *this;
}


PainterBase & Painter::fillRectangle(int x, int y, int w, int h,
				 LColor::color col)
{
	XFillRectangle(display, owner.getPixmap(),
		       lyxColorHandler->getGCForeground(col), x, y, w, h);
	return *this;
}


PainterBase & Painter::fillPolygon(int const * xp, int const * yp, int np,
			       LColor::color col)
{
	boost::scoped_array<XPoint> points(new XPoint[np]);

	for (int i = 0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

	XFillPolygon(display, owner.getPixmap(),
		     lyxColorHandler->getGCForeground(col), points.get(), np, 
		     Nonconvex, CoordModeOrigin);

	return *this;
}      


PainterBase & Painter::arc(int x, int y,
		  unsigned int w, unsigned int h,
		  int a1, int a2, LColor::color col)
{
        XDrawArc(display, owner.getPixmap(),
		 lyxColorHandler->getGCForeground(col),
                 x, y, w, h, a1, a2);
	return *this;
}     


/// Draw lines from x1,y1 to x2,y2. They are arrays
PainterBase & Painter::segments(int const * x1, int const * y1, 
			    int const * x2, int const * y2, int ns,
			    LColor::color col,
			    enum line_style ls, enum line_width lw)
{
	boost::scoped_array<XSegment> s(new XSegment[ns]);

	for (int i = 0; i < ns; ++i) {
		s[i].x1 = x1[i];
		s[i].y1 = y1[i];
		s[i].x2 = x2[i];
		s[i].y2 = y2[i];
	}
	XDrawSegments(display, owner.getPixmap(),
		      lyxColorHandler->getGCLinepars(ls, lw, col),
		      s.get(), ns);

	return *this;
}


PainterBase & Painter::pixmap(int x, int y, int w, int h, Pixmap bitmap)
{
	XGCValues val;
	val.function = GXcopy;
	GC gc = XCreateGC(display, owner.getPixmap(),
			  GCFunction, &val);
	XCopyArea(display, bitmap, owner.getPixmap(), gc,
		  0, 0, w, h, x, y);
	XFreeGC(display, gc);
	return *this;
}


PainterBase & Painter::image(int x, int y, int w, int h, LyXImage const * image)
{
	Pixmap bitmap = image->getPixmap();

	return pixmap(x, y, w, h, bitmap);
}


PainterBase & Painter::text(int x, int y, string const & s, LyXFont const & f)
{
	return text(x, y, s.data(), s.length(), f);
}


PainterBase & Painter::text(int x, int y, char c, LyXFont const & f)
{
	char s[2] = { c, '\0' };
	return text(x, y, s, 1, f);
}


PainterBase & Painter::text(int x, int y, char const * s, size_t ls,
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
		text(x , y, xs.get(), ls, font);
		return *this;
	}

	GC gc = lyxColorHandler->getGCForeground(f.realColor());
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		lyxfont::XSetFont(display, gc, f);
		XDrawString(display, owner.getPixmap(), gc, x, y, s, ls);
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		int tmpx = x;
		for (size_t i = 0; i < ls; ++i) {
			char c = s[i];
			if (islower(static_cast<unsigned char>(c))) {
				c = toupper(c);
				lyxfont::XSetFont(display, gc, smallfont);
				XDrawString(display, owner.getPixmap(),
					    gc, tmpx, y, &c, 1);
				tmpx += lyxfont::XTextWidth(smallfont, &c, 1);
			} else {
				lyxfont::XSetFont(display, gc, f);
				XDrawString(display, owner.getPixmap(),
					    gc, tmpx, y, &c, 1);
				tmpx += lyxfont::XTextWidth(f, &c, 1);
			}
		}
	}

	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, lyxfont::width(s, ls, f));
	}
	
	return *this;
}


PainterBase & Painter::text(int x, int y, XChar2b const * s, int ls,
			LyXFont const & f)
{
	GC gc = lyxColorHandler->getGCForeground(f.realColor());
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		lyxfont::XSetFont(display, gc, f);
		XDrawString16(display, owner.getPixmap(), gc, x, y, s, ls);
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		static XChar2b c;
		int tmpx = x;
		for (int i = 0; i < ls; ++i) {
			if (s[i].byte1 == 0 && islower(s[i].byte2)) {
				c.byte2 = toupper(s[i].byte2);
				lyxfont::XSetFont(display, gc, smallfont);
				XDrawString16(display, owner.getPixmap(),
					    gc, tmpx, y, &c, 1);
				tmpx += lyxfont::XTextWidth16(smallfont, &c, 1);
			} else {
				lyxfont::XSetFont(display, gc, f);
				XDrawString16(display, owner.getPixmap(),
					    gc, tmpx, y, &s[i], 1);
				tmpx += lyxfont::XTextWidth16(f, const_cast<XChar2b *>(&s[i]), 1);
			}
		}
	}
	
	if (f.underbar() == LyXFont::ON) {
		underline(f, x, y, lyxfont::width(s, ls, f));
	}
	
	return *this;
}


void Painter::underline(LyXFont const & f, int x, int y, int width)
{
	int const below = max(lyxfont::maxDescent(f) / 2, 2);
	int const height = max((lyxfont::maxDescent(f) / 4) - 1, 1);
	if (height < 2)
		line(x, y + below, x + width, y + below, f.color());
	else
		fillRectangle(x, y + below, width, below + height,
			      f.color());
}
