// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2000 The LyX Team
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cmath>

#ifdef USE_STL_MEMORY
#include <memory>
#endif

#include FORMS_H_LOCATION
#include "Painter.h"
#include "LString.h"
#include "debug.h"
#include "lyxfont.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "WorkArea.h"

using std::endl;

Painter::Painter(WorkArea & wa)
	: PainterBase(wa)
{
	colormap = fl_state[fl_get_vclass()].colormap;
	// Clear the GC cache
	for (int i = 0; i <= LColor::ignore; ++i) {
		colorGCcache[i] = 0;
	}
}


Painter::~Painter() {
	// Release all the registered GCs
	for (int i = 0; i <= LColor::ignore; ++i) {
		if (colorGCcache[i] != 0) {
			XFreeGC(display, colorGCcache[i]);
		}
	}
	// Iterate over the line cache and Free the GCs
	for (LineGCCache::iterator lit = lineGCcache.begin();
	     lit != lineGCcache.end(); ++lit) {
		XFreeGC(display, (*lit).second);
	}
}


Drawable Painter::drawable() const 
{
	return owner.getPixmap();
}


/* Basic drawing routines */

extern bool Lgb_bug_find_hack;

PainterBase & Painter::point(int x, int y, LColor::color c)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "point not called from "
				"workarea::workhandler\n";
		lyxerr.debug() << "Painter drawable: " << drawable() << endl;
	}
	
	XDrawPoint(display, drawable(), getGCForeground(c), x, y);
	return *this;
}


PainterBase & Painter::line(int x1, int y1, int x2, int y2,
			LColor::color col,
			enum line_style ls,
			enum line_width lw)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "line not called from "
				"workarea::workhandler\n";
		lyxerr.debug() << "Painter drawable: " << drawable() << endl;
	}
	
	XDrawLine(display, drawable(), 
		  getGCLinepars(ls, lw, col), x1, y1, x2, y2);
	return *this;
}


PainterBase & Painter::lines(int const * xp, int const * yp, int np,
			LColor::color col,
			enum line_style ls,
			enum line_width lw)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "lines not called from "
				"workarea::workhandler\n";
		lyxerr.debug() << "Painter drawable: " << drawable() << endl;
	}
	
#ifndef HAVE_AUTO_PTR
	XPoint * points = new XPoint[np];
#else
	auto_ptr<XPoint> points(new Xpoint[np]);
#endif
	for (int i=0; i<np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

        XDrawLines(display, drawable(), getGCLinepars(ls, lw, col), 
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
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "rectangle not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
	XDrawRectangle(display, drawable(), getGCLinepars(ls, lw, col), 
		       x, y, w, h);
	return *this;
}


PainterBase & Painter::fillRectangle(int x, int y, int w, int h,
				 LColor::color col)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "fillrectangle not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
	XFillRectangle(display, drawable(), getGCForeground(col), x, y, w, h);
	return *this;
}


PainterBase & Painter::fillPolygon(int const * xp, int const * yp, int np,
			       LColor::color col)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr <<"fillpolygon not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
#ifndef HAVE_AUTO_PTR
	XPoint * points = new XPoint[np];
#else
	auto_ptr<XPoint> points(new XPoint[np]);
#endif
	for (int i=0; i<np; ++i) {
		points[i].x = xp[i];
		points[i].y = yp[i];
	}

	XFillPolygon(display, drawable(), getGCForeground(col), points, np, 
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
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "arc not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
        XDrawArc(display, drawable(), getGCForeground(col),
                 x, y, w, h, a1, a2);
	return *this;
}     


/// Draw lines from x1,y1 to x2,y2. They are arrays
PainterBase & Painter::segments(int const * x1, int const * y1, 
			    int const * x2, int const * y2, int ns,
			    LColor::color col,
			    enum line_style ls, enum line_width lw)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "segments not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
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
	XDrawSegments(display, drawable(), getGCLinepars(ls, lw, col), s, ns);

#ifndef HAVE_AUTO_PTR
	delete [] s;
#endif
	return *this;
}


PainterBase & Painter::pixmap(int x, int y, int w, int h, Pixmap bitmap)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "workAreaExpose not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
	XGCValues val;
	val.function = GXcopy;
	GC gc = XCreateGC(display, drawable(),
			  GCFunction, &val);
	XCopyArea(display, bitmap, drawable(), gc,
		  0, 0, w, h, x, y);
	XFreeGC(display, gc);
	return *this;
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


PainterBase & Painter::text(int x, int y, char const * s, int ls,
			LyXFont const & f)
{
	if (lyxerr.debugging()) {
		if (!Lgb_bug_find_hack)
			lyxerr << "text not called from "
				"workarea::workhandler\n";
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
	GC gc = getGCForeground(f.realColor());
	XSetFont(display, gc, f.getFontID());
	XDrawString(display, drawable(), gc, x, y, s, ls);
	underline(f, x, y, this->width(s, ls, f));
	return *this;
}


void Painter::underline(LyXFont const & f, int x, int y, int width)
{
	// What about underbars?
	if (f.underbar() == LyXFont::ON && f.latex() != LyXFont::ON) {
		int below = f.maxDescent() / 2;
		if (below < 2) below = 2;
		int height = (f.maxDescent() / 4) - 1;
		if (height < 0) height = 0;
		fillRectangle(x, y + below, width, below + height, f.color());
	}
}


// Gets GC according to color
// Uses caching
GC Painter::getGCForeground(LColor::color c)
{
	if (lyxerr.debugging()) {
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
    	if (colorGCcache[c] != 0) return colorGCcache[c];

	XColor xcol, ccol;
	string s = lcolor.getX11Name(c);
	XGCValues val;

	// Look up the RGB values for the color, and an approximate
	// color that we can hope to get on this display.
        if (XLookupColor(display, colormap, s.c_str(), &xcol, &ccol) == 0) {
		lyxerr << _("LyX: Unknown X11 color ") << s
		       << _(" for ") << lcolor.getGUIName(c) << '\n'
		       << _("     Using black instead, sorry!.") << endl;
		unsigned long bla = BlackPixel(display,
					       DefaultScreen(display));
		val.foreground = bla;
	// Try the exact RGB values first, then the approximate.
	} else if (XAllocColor(display, colormap, &xcol) != 0) {
		if (lyxerr.debugging()) {
			lyxerr << _("LyX: X11 color ") << s
			       << _(" allocated for ") 
			       << lcolor.getGUIName(c) << endl;
		}
		val.foreground = xcol.pixel;
	} else if (XAllocColor(display, colormap, &ccol)) {
		lyxerr << _("LyX: Using approximated X11 color ") << s
		       << _(" allocated for ")
		       << lcolor.getGUIName(c) << endl;
		val.foreground = xcol.pixel;
	} else {
		// Here we are traversing the current colormap to find
		// the color closest to the one we want.
		Visual * vi = DefaultVisual(display, DefaultScreen(display));

		XColor * cmap = new XColor[vi->map_entries];

		for(int i = 0; i < vi->map_entries; ++i) {
			cmap[i].pixel = i;
		}
		XQueryColors(display, colormap, cmap, vi->map_entries);

		// Walk through the cmap and look for close colors.
		int closest_pixel = 0;
		double closest_distance = 1e20; // we want to minimize this
		double distance = 0;
		for(int t = 0; t < vi->map_entries; ++t) {
			// The Euclidean distance between two points in 
			// a three-dimensional space, the RGB color-cube,
			// is used as the distance measurement between two
			// colors.

			// Since square-root is monotonous, we don't have to
			// take the square-root to find the minimum, and thus 
			// we use the squared distance instead to be faster.

			// If we want to get fancy, we could convert the RGB
			// coordinates to a different color-cube, maybe HSV,
			// but the RGB cube seems to work great.  (Asger)
			distance = pow(cmap[t].red   - xcol.red,   2.0) +
				   pow(cmap[t].green - xcol.green, 2.0) +
				   pow(cmap[t].blue  - xcol.blue,  2.0);
			if (distance < closest_distance) {
				closest_distance = distance;
				closest_pixel = t;
			}
		}
		lyxerr << _("LyX: Couldn't allocate '") << s 
		       << _("' for ") << lcolor.getGUIName(c)
		       << _(" with (r,g,b)=(") 
		       << xcol.red << "," << xcol.green << ","
		       << xcol.blue << ").\n"
		       << _("     Using closest allocated "
			    "color with (r,g,b)=(") 
		       << cmap[closest_pixel].red << ","
		       << cmap[closest_pixel].green << ","
		       << cmap[closest_pixel].blue << ") instead.\n"
		       << "Pixel [" << closest_pixel << "] is used." << endl;
		val.foreground = cmap[closest_pixel].pixel;
		delete[] cmap;
	}

	val.function = GXcopy;
	return colorGCcache[c] = XCreateGC(display, drawable(),
				    GCForeground | GCFunction, &val);
}


// Gets GC for line
GC Painter::getGCLinepars(enum line_style ls,
			  enum line_width lw, LColor::color c)
{
	if (lyxerr.debugging()) {
		lyxerr << "Painter drawable: " << drawable() << endl;
	}
	
	int index = lw + (ls << 1) + (c << 3);

	if (lineGCcache.find(index) != lineGCcache.end())
		return lineGCcache[index];

	XGCValues val;
	XGetGCValues(display, getGCForeground(c), GCForeground, &val);
	
	switch (lw) {
	case line_thin:  	val.line_width = 0; break;
	case line_thick: 	val.line_width = 2; break;
	}
	
	switch (ls) {
	case line_solid:	val.line_style = LineSolid; break;
	case line_onoffdash: 	val.line_style = LineOnOffDash; break;
	case line_doubledash: 	val.line_style = LineDoubleDash; break;
	}


	val.cap_style = CapRound;
	val.join_style = JoinRound;
	val.function = GXcopy;

	return lineGCcache[index] =
		XCreateGC(display, drawable(), 
			  GCForeground | GCLineStyle | GCLineWidth | 
			  GCCapStyle | GCJoinStyle | GCFunction, &val);
}
