// -*- C++ -*-
/**
 * \file xfont_metrics.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef XFONT_METRICS_H
#define XFONT_METRICS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "font_metrics.h"
 
#include <X11/Xlib.h>
 
class LyXFont;

namespace xfont_metrics {
	int XTextWidth(LyXFont const & f, char const * str, int count);
	///
	int width(XChar2b const * s, int n, LyXFont const & f);
	///
	int XTextWidth16(LyXFont const & f, XChar2b const * str, int count);
	///
	void XSetFont(Display * display, GC gc, LyXFont const & f);
} // namespace xfont_metrics
 
#endif // FONT_METRICS_H
