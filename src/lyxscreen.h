// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team
 *
 *======================================================*/

#ifndef LYXSCREEN_H
#define LYXSCREEN_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include <X11/Xlib.h>
#include "lyxdraw.h"

class LyXText;
struct Row;
typedef unsigned short Dimension;

/** The class LyXScreen is used for the main Textbody.
    Concretely, the screen is held in a pixmap.  This pixmap is kept up to
    date and used to optimize drawing on the screen.
    This class also handles the drawing of the cursor and partly the selection.
 */
class LyXScreen {
public:
	///
	LyXScreen(Window window,
		  Dimension width, 
		  Dimension height,
		  Dimension offset_x,
		  Dimension offset_y,
		  LyXText *text_ptr);
	///
	~LyXScreen();

	/** Return the forground pixmap. This function is a _hack_,
	  we should be rid of it as soon as possible. But to do that
	  a lot in the mathcode and the figinset has to be rewritten.
	  Tasks for 0.13. */
	Pixmap getForeground() { return foreground; };
	
	/** Draws the screen form textposition y. Uses as much of
	  the already printed pixmap as possible */
	void Draw(long y );

	/// Redraws the screen, without using existing pixmap
	void Redraw();
   
	/// Returns a new top so that the cursor is visible
	long TopCursorVisible();
	/// Redraws the screen such that the cursor is visible
	int FitCursor();
	///
	void ShowCursor();
	///
	void HideCursor();
	///
	void CursorToggle();
	///
	void ShowManualCursor(long x, long y, int asc, int desc);
	///
	void HideManualCursor(long x, long y, int asc, int desc);
	/// returns 1 if first has changed, otherwise 0
	int  FitManualCursor(long, long, int, int);
	///
	void ToggleSelection(bool = true);
	///
	void ToggleToggle();
	
	/** Updates part of the screen. If text->status is
	  LyXText::NEED_MORE_REFRESH, we update from the
	  point of change and to the end of the screen.
	  If text->status is LyXText::NEED_VERY_LITTLE_REFRESH,
	  we only update the current row. */
	void Update();

	/** Updates part of the screen. Updates till row with cursor,
	  or only current row */
	void SmallUpdate();

	/** Functions for drawing into the LyXScreen. The number of
	  drawing functions should be minimized, now there
	  is too many. And also there is mixed X and XForms drawing
	  functions called. Not good. */
	void drawPoint(GC gc, int x, int y);
	///
	void drawLine(gc_type t, int baseline, int x, int length);
	///
	void drawLine(GC gc, int a, int b, int c, int d);
	///
	void drawLines(GC gc, XPoint *p, int np);
	///
	void drawVerticalLine(gc_type t, int x, int y1, int y2);
	///
	void drawOnOffLine(int baseline, int x, int length);
	///
	void drawThickLine(int baseline, int x, int length);
	///
	void drawTableLine(int baseline, int x, int length, bool on_off);
	///
	void drawVerticalTableLine(int x, int y1, int y2, bool on_off);
	///
	void drawVerticalOnOffLine(int x, int y1, int y2);
	///
	void drawArc(GC gc, int x, int y,
		     unsigned int w, unsigned int h,
		     int a1, int a2);
	///
	void drawSegments(GC gc, XSegment *s, int ns);
	///
	void fillRectangle(gc_type t, int, int, int, int);
	///
	void drawRectangle(gc_type t, int x, int y, int width, int height);
	///
	void drawFrame(int ft, int x, int y, int w, int h,
		       FL_COLOR col, int b); 
	///
	int drawText(LyXFont const &font, char const*,
		      int n, int baseline, int x);
	///
	int drawString(LyXFont const &font, string const &str,
			int baseline, int x);
		
	/// first visible pixel-row
	long first;

	///
	bool cursor_visible;
private:
	/// Copies specified area of pixmap to screen
	void expose(int x, int y, int exp_width, int exp_height); 

	/// y1 and y2 are coordinates of the screen
	void DrawFromTo(int y1, int y2);
   
	/// y is a coordinate of the text
	void DrawOneRow(Row* row, long &y_text);

	///
	LyXText *text;

	///
	Pixmap foreground;
	///
	Pixmap cursor_pixmap;
	///
	int cursor_pixmap_x;
	///
	int cursor_pixmap_y;
	///
	int cursor_pixmap_w;
	///
	int cursor_pixmap_h;
	///
	Window _window;
        ///
	Dimension _width;
	///
	Dimension _height;
        ///
	Dimension _offset_x;
	///
	Dimension _offset_y;
	///
	long screen_refresh_y;
	///
	Row *screen_refresh_row;
	///
	friend class InsetFormula;  
};

// Some of the easy to inline draw methods:

inline void LyXScreen::drawPoint(GC gc, int x, int y)
{
	XDrawPoint(fl_display, foreground, gc,
		   x, y);
}


inline void LyXScreen::drawLine(GC gc, int a, int b, int c, int d)
{
	XDrawLine(fl_display, foreground, gc, a, b, c, d);
}


inline void LyXScreen::drawLine(gc_type t, int baseline, int x, int length)
{
	drawLine(getGC(t), x, baseline, x + length, baseline);
}


inline void LyXScreen::drawLines(GC gc, XPoint *p, int np)
{
	XDrawLines(fl_display, foreground, gc, p, np, CoordModeOrigin);
}


inline void LyXScreen::drawVerticalLine(gc_type t, int x, int y1, int y2)
{
	drawLine(getGC(t),
		 x,
		 y1,
		 x,
		 y2);
}


inline void LyXScreen::drawOnOffLine(int baseline, int x, int length)
{
	drawLine(getGC(gc_on_off_line),
		 x,
		 baseline,
		 x + length,
		 baseline);
}

	
inline void LyXScreen::drawThickLine(int baseline, int x, int length)
{
	drawLine(getGC(gc_thick_line),
		 x,
		 baseline,
		 x + length,
		 baseline);
}


inline void LyXScreen::drawVerticalOnOffLine(int x, int y1, int y2)
{
	drawLine(getGC(gc_fill),
		 x,
		 y1,
		 x,
		 y2);
}	


inline void LyXScreen::drawArc(GC gc, int x, int y,
			unsigned int w, unsigned int h,
			int a1, int a2)
{
	XDrawArc(fl_display, foreground, gc,
		 x, y,
		 w, h, a1, a2);
}


inline void LyXScreen::drawSegments(GC gc, XSegment *s, int ns)
{
	XDrawSegments(fl_display, foreground, gc, s, ns);
}


inline void LyXScreen::fillRectangle(gc_type t, int a, int b, int c, int d)
{
	XFillRectangle(fl_display, foreground, getGC(t),
		       a, b, c, d);
}


inline void LyXScreen::drawRectangle(gc_type t, int x, int y, int width, int height)
{
	XDrawRectangle(fl_display, foreground, getGC(t),
		       x, y, width, height);
}


inline int LyXScreen::drawText(LyXFont const &font, char const*fs,
			 int n, int baseline, int x)
{
	return font.drawText(fs, n, foreground, baseline, x);
}


inline int LyXScreen::drawString(LyXFont const &font, string const &str,
			   int baseline, int x)
{
	return font.drawString(str, foreground, baseline, x);
}

#endif
