// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995-2001 The LyX Team
 *
 * ======================================================*/

#ifndef PAINTER_H
#define PAINTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

// This is only included to provide stuff for the non-public sections
#include <X11/Xlib.h>

#include "PainterBase.h"

class LyXFont;
class WorkArea;

/** An inplementation for the X Window System. Xlib.
    
    Classes similar to this one can be made for gtk+, Qt, etc.
*/
class Painter : public PainterBase {
public:
	/// Constructor 
	explicit Painter(WorkArea &);
	
	/// Draw a line from point to point
	PainterBase & line(int x1, int y1, int x2, int y2, 
			   LColor::color = LColor::foreground,
			   enum line_style = line_solid,
			   enum line_width = line_thin);
	
	/// Here xp and yp are arrays of points
	PainterBase & lines(int const * xp, int const * yp, int np,
			    LColor::color = LColor::foreground,
			    enum line_style = line_solid,
			    enum line_width = line_thin);
	
	/// Here xp and yp are arrays of points
	PainterBase & fillPolygon(int const * xp, int const * yp, int np,
				  LColor::color = LColor::foreground);
	
	/// Draw lines from x1,y1 to x2,y2. They are arrays
        PainterBase & segments(int const * x1, int const * y1, 
			       int const * x2, int const * y2, int ns,
			       LColor::color = LColor::foreground,
			       enum line_style = line_solid,
			       enum line_width = line_thin);
	
	/// Draw a rectangle 
	PainterBase & rectangle(int x, int y, int w, int h,
				LColor::color = LColor::foreground,
				enum line_style = line_solid,
				enum line_width = line_thin);
	
	/// Draw an arc
	PainterBase & arc(int x, int y, unsigned int w, unsigned int h, 
			  int a1, int a2,
			  LColor::color = LColor::foreground);
	
	/// Draw a pixel
	PainterBase & point(int x, int y, LColor::color = LColor::foreground);
	
	/// Fill a rectangle
	PainterBase & fillRectangle(int x, int y, int w, int h,
				    LColor::color);
	
	/// For the graphics inset.
	PainterBase & image(int x, int y, int w, int h, LyXImage const * image);
	
	/// For the figinset
	PainterBase & pixmap(int x, int y, int w, int h, Pixmap bitmap);
	
	/// Draw a string at position x, y (y is the baseline)
	PainterBase & text(int x, int y,
			   string const & str, LyXFont const & f);
	
	/** Draw a string at position x, y (y is the baseline)
	    This is just for fast drawing */
	PainterBase & text(int x, int y, char const * str, size_t l,
			   LyXFont const & f);
	
	/// Draw a char at position x, y (y is the baseline)
	PainterBase & text(int x, int y, char c, LyXFont const & f);

	/// Draw a wide string at position x, y
	PainterBase & text(int x, int y, XChar2b const * str, int l,
		   LyXFont const & f);	
private:
	/// Check the font, and if set, draw an underline
	void underline(LyXFont const & f, int x, int y, int width);
	
	///
	Display * display;
};

#endif
