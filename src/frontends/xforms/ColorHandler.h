// -*- C++ -*-
/**
 * \file ColorHandler.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COLOR_HANDLER_H
#define COLOR_HANDLER_H

#include "frontends/Painter.h"

#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>

// This is only included to provide stuff for the non-public sections
#include <X11/Xlib.h>

class LColor_color;
class LyXFont;

namespace lyx {
namespace frontend {

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
	unsigned long colorPixel(LColor_color c);
	///
	GC getGCForeground(LColor_color c);
	///
	GC getGCLinepars(Painter::line_style,
			 Painter::line_width, LColor_color c);
	/// update the cache after a color definition change
	void updateColor(LColor_color c);

private:
	///
	Display * display;
	///
	Colormap colormap;
	///
	std::vector<GC> colorGCcache;
	///
	GC getGCForeground(std::string const & s);
	///
	typedef std::map<int, GC> LineGCCache;
	///
	LineGCCache lineGCcache;
	///
	Pixmap drawable;
};

///
extern boost::scoped_ptr<LyXColorHandler> lyxColorHandler;

} // namespace frontend
} // namespace lyx

#endif
