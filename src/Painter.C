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

#ifdef USE_STL_MEMORY
#include <memory>
#endif

#include <cmath>

#include FORMS_H_LOCATION
#include "Painter.h"
#include "LString.h"
#include "debug.h"
#include "lyxfont.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "WorkArea.h"
#include "font.h"
#include "ColorHandler.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"

#include "frontends/support/LyXImage.h"

using std::endl;
using std::max;

Painter::Painter(WorkArea & wa)
	: PainterBase(wa)
{
	display = fl_get_display();
}


/* Basic drawing routines */

extern bool Lgb_bug_find_hack;

PainterBase & Painter::point(int x, int y, LColor::color c)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "point not called from "
				"workarea::workhandler\n";
		lyxerr[Debug::INFO] << "Painter drawable: "
			       << owner.getPixmap() << endl;
	}
	
	XDrawPoint(display, owner.getPixmap(),
		   lyxColorHandler->getGCForeground(c), x, y);
	return *this;
}


PainterBase & Painter::line(int x1, int y1, int x2, int y2,
			LColor::color col,
			enum line_style ls,
			enum line_width lw)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "line not called from "
				"workarea::workhandler\n";
		lyxerr[Debug::INFO] << "Painter drawable: "
			       << owner.getPixmap() << endl;
	}
	
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
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "lines not called from "
				"workarea::workhandler\n";
		lyxerr[Debug::INFO] << "Painter drawable: "
			       << owner.getPixmap() << endl;
	}
	
#ifndef HAVE_AUTO_PTR
	XPoint * points = new XPoint[np];
#else
	auto_ptr<XPoint> points(new Xpoint[np]);
#endif
	for (int i = 0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

        XDrawLines(display, owner.getPixmap(),
		   lyxColorHandler->getGCLinepars(ls, lw, col), 
		   points, np, CoordModeOrigin);

#ifndef HAVE_AUTO_PTR
	delete[] points;
#endif	
	return *this;
}      


PainterBase & Painter::rectangle(int x, int y, int w, int h,
				 LColor::color col,
				 enum line_style ls,
				 enum line_width lw)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "rectangle not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: "
		       << owner.getPixmap() << endl;
	}
	
	XDrawRectangle(display, owner.getPixmap(),
		       lyxColorHandler->getGCLinepars(ls, lw, col), 
		       x, y, w, h);
	return *this;
}


PainterBase & Painter::fillRectangle(int x, int y, int w, int h,
				 LColor::color col)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "fillrectangle not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: "
		       << owner.getPixmap() << endl;
	}
	
	XFillRectangle(display, owner.getPixmap(),
		       lyxColorHandler->getGCForeground(col), x, y, w, h);
	return *this;
}


PainterBase & Painter::fillPolygon(int const * xp, int const * yp, int np,
			       LColor::color col)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr <<"fillpolygon not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << owner.getPixmap() << endl;
	}
	
#ifndef HAVE_AUTO_PTR
	XPoint * points = new XPoint[np];
#else
	auto_ptr<XPoint> points(new XPoint[np]);
#endif
	for (int i=0; i < np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

	XFillPolygon(display, owner.getPixmap(),
		     lyxColorHandler->getGCForeground(col), points, np, 
		     Nonconvex, CoordModeOrigin);
#ifndef HAVE_AUTO_PTR
	delete[] points;
#endif	
	return *this;
}      


PainterBase & Painter::arc(int x, int y,
		  unsigned int w, unsigned int h,
		  int a1, int a2, LColor::color col)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "arc not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << owner.getPixmap() << endl;
	}
	
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
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "segments not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << owner.getPixmap() << endl;
	}
	
#ifndef HAVE_AUTO_PTR
	XSegment * s= new XSegment[ns];
#else
	auto_ptr<XSegment> s(new XSegment[ns]);
#endif
	for (int i=0; i<ns; ++i) {
		s[i].x1 = x1[i];
		s[i].y1 = y1[i];
		s[i].x2 = x2[i];
		s[i].y2 = y2[i];
	}
	XDrawSegments(display, owner.getPixmap(),
		      lyxColorHandler->getGCLinepars(ls, lw, col), s, ns);

#ifndef HAVE_AUTO_PTR
	delete [] s;
#endif
	return *this;
}

PainterBase & Painter::pixmap(int x, int y, int w, int h, Pixmap bitmap)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "workAreaExpose not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << owner.getPixmap() << endl;
	}

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
	return text(x, y, s.c_str(), s.length(), f);
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
		XChar2b * xs = new XChar2b[ls];
		Encoding const * encoding = f.language()->encoding();
		LyXFont font(f);
		if (f.family() == LyXFont::SYMBOL_FAMILY) {
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
		text(x , y, xs, ls, font);
		delete[] xs;
		return *this;
	}

	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "text not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << owner.getPixmap() << endl;
	}
	GC gc = lyxColorHandler->getGCForeground(f.realColor());
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		lyxfont::XSetFont(display, gc, f);
		XDrawString(display, owner.getPixmap(), gc, x, y, s, ls);
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		char c;
		int tmpx = x;
		for (size_t i = 0; i < ls; ++i) {
			c = s[i];
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
	if (f.underbar() == LyXFont::ON
#ifndef NO_LATEX
	    && f.latex() != LyXFont::ON
#endif
		)
		underline(f, x, y, lyxfont::width(s, ls, f));
	return *this;
}


PainterBase & Painter::text(int x, int y, XChar2b const * s, int ls,
			LyXFont const & f)
{
	if (lyxerr.debugging(Debug::GUI)) {
		if (!Lgb_bug_find_hack)
			lyxerr << "text not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << owner.getPixmap() << endl;
	}
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
	if (f.underbar() == LyXFont::ON
#ifndef NO_LATEX
	    && f.latex() != LyXFont::ON
#endif
		)
		underline(f, x, y, lyxfont::width(s, ls, f));
	return *this;
}


void Painter::underline(LyXFont const & f, int x, int y, int width)
{
	int below = max(lyxfont::maxDescent(f) / 2, 2);
	int height = max((lyxfont::maxDescent(f) / 4) - 1, 1);
	if (height < 2)
		line(x, y + below, x + width, y + below, f.color());
	else
		fillRectangle(x, y + below, width, below + height,
			      f.color());
}
