// -*- C++ -*-
/**
 * \file xfont_metrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
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
