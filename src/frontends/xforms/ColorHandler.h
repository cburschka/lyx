// -*- C++ -*-
/**
 * \file ColorHandler.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef COLOR_HANDLER_H
#define COLOR_HANDLER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/Painter.h"

#include <boost/scoped_ptr.hpp>

// This is only included to provide stuff for the non-public sections
#include <X11/Xlib.h>

class LyXFont;

/**
 * This is a factory class that can produce GCs with a specific
 * color. It will cache GCs for performance.
 */
class LyXColorHandler {
public:
	///
	LyXColorHandler();
	///
	~LyXColorHandler();
	///
	unsigned long colorPixel(LColor::color c);
	///
	GC getGCForeground(LColor::color c);
	///
	GC getGCLinepars(Painter::line_style,
			 Painter::line_width, LColor::color c);
	/// update the cache after a color definition change
	void updateColor(LColor::color c);

private:
	///
	Display * display;
	///
	Colormap colormap;
	///
	GC colorGCcache[LColor::ignore + 1];
	///
	typedef std::map<int, GC> LineGCCache;
	///
	LineGCCache lineGCcache;
	///
	Pixmap drawable;
};

///
extern boost::scoped_ptr<LyXColorHandler> lyxColorHandler;

#endif
