// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
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

//namespace lyx {
//namespace font {
///
struct lyxfont {
	///
	static
	int maxAscent(LyXFont const & f);
	///
	static
	int maxDescent(LyXFont const & f);
	///
	static
	int ascent(char c, LyXFont const & f);
	///
	static
	int descent(char c, LyXFont const & f);
	///
	static
	int lbearing(char c, LyXFont const & f);
	///
	static
	int rbearing(char c, LyXFont const & f);
	///
	static
	int width(char c, LyXFont const & f) {
		return width(&c, 1, f);
	}
	///
	static
	int width(char const * s, int n, LyXFont const & f);
	///
	static
	int width(string const & s, LyXFont const & f) {
		if (s.empty()) return 0;
		return width(s.c_str(), s.length(), f);
	}
	///
	static
	int width(char const * s, LyXFont const & f) {
		return width(s, strlen(s), f);
	}
	///
	static
	int signedWidth(string const & s, LyXFont const & f);
	///
	static
	int XTextWidth(LyXFont const & f, char * str, int count);
	///
	static
	int width(XChar2b const * s, int n, LyXFont const & f);
	///
	static
	int XTextWidth16(LyXFont const & f, XChar2b * str, int count);
	///
	static
	void XSetFont(Display * display, GC gc, LyXFont const & f);
};

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
