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

#include FORMS_H_LOCATION
#include "debug.h"

#include "ColorHandler.h"
#include "gettext.h"

using std::endl;

LyXColorHandler::LyXColorHandler() 
{
	display = fl_display;
	drawable = XCreatePixmap(display, fl_root, 10, 10,
				 fl_get_visual_depth());
	
	colormap = fl_state[fl_get_vclass()].colormap;
	// Clear the GC cache
	for (int i = 0; i <= LColor::ignore; ++i) {
		colorGCcache[i] = 0;
	}
}


LyXColorHandler::~LyXColorHandler()
{
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


unsigned long LyXColorHandler::colorPixel(LColor::color c)
{
	XGCValues val;
	XGetGCValues(display, getGCForeground(c), GCForeground, &val);
	return val.foreground;
}


// Gets GC according to color
// Uses caching
GC LyXColorHandler::getGCForeground(LColor::color c)
{
	//if (lyxerr.debugging()) {
	//	lyxerr << "Painter drawable: " << drawable() << endl;
	//}
	
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
		       << cmap[closest_pixel].blue << _(") instead.\n")
		       << _("Pixel [") << closest_pixel << _("] is used.") 
		       << endl;
		val.foreground = cmap[closest_pixel].pixel;
		delete[] cmap;
	}

	val.function = GXcopy;
	return colorGCcache[c] = XCreateGC(display, drawable,
				    GCForeground | GCFunction, &val);
}


// Gets GC for line
GC LyXColorHandler::getGCLinepars(PainterBase::line_style ls,
			  PainterBase::line_width lw, LColor::color c)
{
	//if (lyxerr.debugging()) {
	//	lyxerr << "Painter drawable: " << drawable() << endl;
	//}
	
	int index = lw + (ls << 1) + (c << 3);

	if (lineGCcache.find(index) != lineGCcache.end())
		return lineGCcache[index];

	XGCValues val;
	XGetGCValues(display, getGCForeground(c), GCForeground, &val);
	
	switch (lw) {
	case PainterBase::line_thin:  	val.line_width = 0; break;
	case PainterBase::line_thick: 	val.line_width = 2; break;
	}
	
	switch (ls) {
	case PainterBase::line_solid:	val.line_style = LineSolid; break;
	case PainterBase::line_onoffdash: 	val.line_style = LineOnOffDash; break;
	case PainterBase::line_doubledash: 	val.line_style = LineDoubleDash; break;
	}


	val.cap_style = CapRound;
	val.join_style = JoinRound;
	val.function = GXcopy;

	return lineGCcache[index] =
		XCreateGC(display, drawable, 
			  GCForeground | GCLineStyle | GCLineWidth | 
			  GCCapStyle | GCJoinStyle | GCFunction, &val);
}

//
LyXColorHandler * lyxColorHandler;
