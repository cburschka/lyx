// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995-2000 The LyX Team
 *
 * ======================================================*/

#ifndef COLOR_HANDLER_H
#define COLOR_HANDLER_H

#ifdef __GNUG__
#pragma interface
#endif

//#include "config.h"
//#include "LString.h"

// This is only included to provide stuff for the non-public sections
#include <X11/Xlib.h>

#include <map>
#include "PainterBase.h"
#include "LColor.h"

class LyXFont;
class WorkArea;

///
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
	GC getGCLinepars(PainterBase::line_style,
			 PainterBase::line_width, LColor::color c);
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

extern LyXColorHandler * lyxColorHandler;

#endif
