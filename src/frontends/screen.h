// -*- C++ -*-
/**
 * \file screen.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */ 

#ifndef LYXSCREEN_H
#define LYXSCREEN_H

#ifdef __GNUG__
#pragma interface
#endif

#include <X11/Xlib.h>

class LyXText;
class LyXCursor;
class WorkArea;
class Buffer;
class BufferView;

struct Row;

/**
 * LScreen - document rendering management
 *
 * This class is used to manage the on-screen rendering inside the
 * work area; it is responsible for deciding which LyXText rows
 * need re-drawing.
 *
 * This class will arrange for LyXText to paint onto a pixmap
 * provided by the WorkArea widget.
 *
 * The blinking cursor is also handled here.
 */
class LScreen {
public:
	///
	enum Cursor_Shape {
		///
		BAR_SHAPE,
		///
		L_SHAPE,
		///
		REVERSED_L_SHAPE
	};

	///
	LScreen(WorkArea &);

	///
	~LScreen();

	void reset();
 
	/// Sets the cursor color to LColor::cursor.
	void setCursorColor();

	/** Draws the screen form textposition y. Uses as much of
	    the already printed pixmap as possible */
	void draw(LyXText *, BufferView *, unsigned int y);

	/// Redraws the screen, without using existing pixmap
	void redraw(LyXText *, BufferView *);

	/**
	 * topCursorVisible - get a new "top" to make the cursor visible
	 * @param c the cursor
	 * @param top_y the current y location of the containing region
	 *
	 * This helper function calculates a new y co-ordinate for
	 * the top of the containing region such that the cursor contained
	 * within the LyXText is "nicely" visible.
	 */ 
	unsigned int topCursorVisible(LyXCursor const & c, int top_y);
 
	/// Redraws the screen such that the cursor is visible
	bool fitCursor(LyXText *, BufferView *);
	///
	void showCursor(LyXText const *, BufferView const *);
	///
	void hideCursor();
	///
	void cursorToggle(BufferView *) const;
	///
	void showManualCursor(LyXText const *, int x, int y,
			      int asc, int desc,
			      Cursor_Shape shape);
	/// returns 1 if first has changed, otherwise 0
	bool fitManualCursor(LyXText *, BufferView *, int, int, int, int);
	///
	void toggleSelection(LyXText *, BufferView *, bool = true,
			     int y_offset = 0, int x_offset = 0);
	///
	void toggleToggle(LyXText *, BufferView *,
			  int y_offset = 0, int x_offset = 0);

	/** Updates part of the screen. If text->status is
	    LyXText::NEED_MORE_REFRESH, we update from the
	    point of change and to the end of the screen.
	    If text->status is LyXText::NEED_VERY_LITTLE_REFRESH,
	    we only update the current row. */
	void update(LyXText *, BufferView *, int y_offset=0, int x_offset=0);
	///
	bool forceClear() const { return force_clear; }

	///
	bool cursor_visible;
private:
	/// Copies specified area of pixmap to screen
	void expose(int x, int y, int exp_width, int exp_height);

	/// y1 and y2 are coordinates of the screen
	void drawFromTo(LyXText *, BufferView *, int y1, int y2,
			int y_offset = 0, int x_offset = 0, bool internal=false);

	/// y is a coordinate of the text
	void drawOneRow(LyXText *, BufferView *, Row * row,
			int y_text, int y_offset = 0, int x_offset = 0);

	///
	WorkArea & owner;

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
	GC gc_copy;
	///
	bool force_clear;
};

#endif
