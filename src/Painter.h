// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1999-2000 The LyX Team
 *
 * ======================================================*/

#ifdef USE_PAINTER

#ifndef PAINTER_H
#define PAINTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "config.h"
#include "LString.h"

// This is only included to provide stuff for the non-public sections
#include <X11/Xlib.h>

#include <map>
#include "PainterBase.h"
#include "LColor.h"

class LyXFont;
class WorkArea;

/** An inplementation for the X Window System. Xlib.
  
    Classes similar to this one can be made for gtk+, Qt, etc.
 */
class Painter : public PainterBase {
public:
	/// Constructor 
	Painter(WorkArea &);

	/// Destructor
	~Painter();
    
	/**@Basic drawing routines */
	/// Draw a line from point to point
	Painter & line(int x1, int y1, int x2, int y2, 
		       LColor::color = LColor::foreground,
		       enum line_style = line_solid,
		       enum line_width = line_thin);

	/// Here xp and yp are arrays of points
	Painter & lines(int const * xp, int const * yp, int np,
			LColor::color = LColor::foreground,
			enum line_style = line_solid,
			enum line_width = line_thin);

	/// Here xp and yp are arrays of points
	Painter & fillPolygon(int const * xp, int const * yp, int np,
			LColor::color = LColor::foreground);

	/// Draw lines from x1,y1 to x2,y2. They are arrays
        Painter & segments(int const * x1, int const * y1, 
			   int const * x2, int const * y2, int ns,
			   LColor::color = LColor::foreground,
			   enum line_style = line_solid,
			   enum line_width = line_thin);

	/// Draw a rectangle 
	Painter & rectangle(int x, int y, int w, int h,
			    LColor::color = LColor::foreground,
			    enum line_style = line_solid,
			    enum line_width = line_thin);

	/// Draw an arc
	Painter & arc(int x, int y, unsigned int w, unsigned int h, 
		      int a1, int a2,
		      LColor::color = LColor::foreground);
	
	/// Draw a pixel
	Painter & point(int x, int y, LColor::color = LColor::foreground);
	
	/// Fill a rectangle
	Painter & fillRectangle(int x, int y, int w, int h,
				LColor::color = LColor::background);
	
	/**@Image stuff */
	
	/// For the figure inset
	Painter & pixmap(int x, int y, Pixmap bitmap);

	/**@String functions */
	
	/// Draw a string at position x, y (y is the baseline)
	Painter & text(int x, int y, string const & str, LyXFont const & f);

	/** Draw a string at position x, y (y is the baseline)
	    This is just for fast drawing */
	Painter & text(int x, int y, char const * str, int l,
		       LyXFont const & f);

	/// Draw a char at position x, y (y is the baseline)
	Painter & text(int x, int y, char c, LyXFont const & f);
	
protected:
	/**@Support for X only, by now */
	friend class WorkArea;
	///
	Painter & setDisplay(Display * d) { display = d; return *this; }

	///
	Painter & setDrawable(Drawable d) { drawable = d; return *this; }

	/// Get foreground color in ordinary GC
	GC getGCForeground(LColor::color c);
	
	/// Set up GC according to line style
	GC getGCLinepars(enum line_style, enum line_width, LColor::color c);

	/// Check the font, and if set, draw an underline
	void underline(LyXFont const & f, int x, int y, int width);

	/**@Low level X parameters */
	///
	Display * display;
	///
	Drawable drawable;
	///
	Colormap colormap;

	/// Caching of ordinary color GCs
	GC colorGCcache[LColor::ignore + 1];
	/// Caching of GCs used for lines
	typedef map<int, GC, less<int> > LineGCCache;
	///
	LineGCCache lineGCcache;
};
#endif

#endif
