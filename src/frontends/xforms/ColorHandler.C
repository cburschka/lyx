/**
 * \file ColorHandler.C
 * Read the file COPYING
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ColorHandler.h"
#include "LColor.h"
#include "gettext.h"
#include "debug.h"

#include <cmath>

#include <boost/scoped_array.hpp>
 
#ifndef CXX_GLOBAL_CSTD
using std::pow;
#endif

using std::endl;

#include FORMS_H_LOCATION

LyXColorHandler::LyXColorHandler()
{
	display = fl_get_display();
	drawable = XCreatePixmap(display,
				 RootWindow(display, fl_screen),
				 10, 10, fl_get_visual_depth());
	
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
		XFreeGC(display, lit->second);
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
	if (colorGCcache[c] != 0)
		return colorGCcache[c];

	XColor xcol;
	XColor ccol;
	string const s = lcolor.getX11Name(c);
	XGCValues val;

	// Look up the RGB values for the color, and an approximate
	// color that we can hope to get on this display.
	if (XLookupColor(display, colormap, s.c_str(), &xcol, &ccol) == 0) {
		lyxerr << _("LyX: Unknown X11 color ") << s
		       << _(" for ") << lcolor.getGUIName(c) << '\n'
		       << _("     Using black instead, sorry!") << endl;
		unsigned long bla = BlackPixel(display,
					       DefaultScreen(display));
		val.foreground = bla;
	// Try the exact RGB values first, then the approximate.
	} else if (XAllocColor(display, colormap, &xcol) != 0) {
		if (lyxerr.debugging(Debug::GUI)) {
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

		boost::scoped_array<XColor> cmap(new XColor[vi->map_entries]);

		for (int i = 0; i < vi->map_entries; ++i) {
			cmap[i].pixel = i;
		}
		XQueryColors(display, colormap, cmap.get(), vi->map_entries);

		// Walk through the cmap and look for close colors.
		int closest_pixel = 0;
		double closest_distance = 1e20; // we want to minimize this
		double distance = 0;
		for (int t = 0; t < vi->map_entries; ++t) {
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
	}

	val.function = GXcopy;
	return colorGCcache[c] = XCreateGC(display, drawable,
				    GCForeground | GCFunction, &val);
}


// Gets GC for line
GC LyXColorHandler::getGCLinepars(Painter::line_style ls,
				  Painter::line_width lw, LColor::color c)
{
	//if (lyxerr.debugging()) {
	//	lyxerr << "Painter drawable: " << drawable() << endl;
	//}
	
	int index = lw + (ls << 1) + (c << 6);

	LineGCCache::iterator it = lineGCcache.find(index);
	if (it != lineGCcache.end())
		return it->second;

	XGCValues val;
	XGetGCValues(display, getGCForeground(c), GCForeground, &val);
	
	switch (lw) {
	case Painter::line_thin:
		val.line_width = 0;
		break;
	case Painter::line_thick:
		val.line_width = 2;
		break;
	}
	
	switch (ls) {
	case Painter::line_solid:
		val.line_style = LineSolid;
		break;
	case Painter::line_onoffdash:
		val.line_style = LineOnOffDash;
		break;
	}


	val.cap_style = CapRound;
	val.join_style = JoinRound;
	val.function = GXcopy;

	return lineGCcache[index] =
		XCreateGC(display, drawable,
			  GCForeground | GCLineStyle | GCLineWidth |
			  GCCapStyle | GCJoinStyle | GCFunction, &val);
}


// update GC cache after color redefinition
void LyXColorHandler::updateColor (LColor::color c)
{
	// color GC cache
	GC gc = colorGCcache[c];
	if (gc != 0) {
		XFreeGC(display, gc);
		colorGCcache[c] = NULL;
		getGCForeground(c);
	}

	// line GC cache

	for (int ls = 0; ls < 3; ++ls)
		for (int lw = 0; lw < 2; ++lw) {
			int const index = lw + (ls << 1) + (c << 6);
			LineGCCache::iterator it = lineGCcache.find(index);
			if (it != lineGCcache.end()) {
				gc = it->second;
				XFreeGC(display, gc);
				lineGCcache.erase(it);
				getGCLinepars(Painter::line_style(ls),
					      Painter::line_width(lw), c);
			}
		}

}

//
boost::scoped_ptr<LyXColorHandler> lyxColorHandler;
