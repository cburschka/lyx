// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef FONT_H
#define FONT_H

#ifdef __GNUG__
#pragma interface
#endif

#include <X11/Xlib.h>

#include "LString.h"

class LyXFont;

namespace font_metrics {
//namespace lyx {
//namespace font {
///
//istruct lyxfont {
	///
	int maxAscent(LyXFont const & f);
	///
	int maxDescent(LyXFont const & f);
	///
	int ascent(char c, LyXFont const & f);
	///
	int descent(char c, LyXFont const & f);
	///
	int lbearing(char c, LyXFont const & f);
	///
	int rbearing(char c, LyXFont const & f);
	///
	int width(char const * s, size_t n, LyXFont const & f);
	///
	int width(char c, LyXFont const & f);
	///
	int width(string const & s, LyXFont const & f);
	///
	//static
	//int width(char const * s, LyXFont const & f) {
	//	return width(s, strlen(s), f);
	//}
	///
	int signedWidth(string const & s, LyXFont const & f);
	///
	int XTextWidth(LyXFont const & f, char const * str, int count);
	///
	int width(XChar2b const * s, int n, LyXFont const & f);
	///
	int XTextWidth16(LyXFont const & f, XChar2b const * str, int count);
	///
	void XSetFont(Display * display, GC gc, LyXFont const & f);
	// A couple of more high-level metrics
	///
	void rectText(string const & str, LyXFont const & font,
		      int & width, int & ascent, int & descent);
	///
	void buttonText(string const & str, LyXFont const & font,
			int & width, int & ascent, int & descent);
//};
}

//} // end of namespace font

// import into namespace lyx
//using font::maxAscent;
//using font::maxDescent;
//using font::ascent;
//using font::descent;
//using font::lbearing;
//using font::rbearing;
//using font::width;
//using font::signedWidth;
//using font::XTextWidth;
//using font::XSetFont;

//} // end of namespace lyx

#endif
