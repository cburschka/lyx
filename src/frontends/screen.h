// -*- C++ -*-
/**
 * \file screen.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef SCREEN_H
#define SCREEN_H

#ifdef __GNUG__
#pragma interface
#endif
 
class LyXText;
class LyXCursor;
class WorkArea;
class BufferView;
struct Row;

/**
 * LyXScreen - document rendering management
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
class LyXScreen {
public:
	/// types of cursor in work area
	enum Cursor_Shape {
		/// normal I-beam
		BAR_SHAPE,
		/// L-shape for locked insets of a different language
		L_SHAPE,
		/// reverse L-shape for RTL text
		REVERSED_L_SHAPE
	};

	LyXScreen();

	virtual ~LyXScreen();
	 
	/**
	 * draw the screen from a given position
	 * @param y the text position to draw from
	 *
	 * Uses as much of the already printed pixmap as possible 
	 */
	virtual void draw(LyXText *, BufferView *, unsigned int y) = 0;
 
	/**
	 * showManualCursor - display the cursor on the work area
	 * @param text the lyx text containing the cursor
	 * @param x the x position of the cursor
	 * @param y the y position of the row's baseline
	 * @param asc ascent of the row
	 * @param desc descent of the row
	 * @param shape the current shape
	 */
	virtual void showManualCursor(LyXText const *, int x, int y,
			      int asc, int desc,
			      Cursor_Shape shape) = 0;
	
	/// unpaint the cursor painted by showManualCursor()
	virtual void hideCursor() = 0;
 
	/**
	 * fit the cursor onto the visible work area, scrolling if necessary
	 * @param bv the buffer view
	 * @param vheight the height of the visible region
	 * @param base_y the top of the lyxtext to look at
	 * @param x the new x position
	 * @param y the new y position
	 * @param a ascent of the cursor's row
	 * @param d descent of the cursor's row
	 * @return true if the work area needs scrolling as a result
	 */
	bool fitManualCursor(BufferView * bv, LyXText * text,
		int x, int y, int a, int d);
 
	/// redraw the screen, without using existing pixmap
	virtual void redraw(LyXText *, BufferView *);
   
	/// draw the cursor if it's not already shown
	virtual void showCursor(LyXText const *, BufferView const *);
 
	/**
	 * topCursorVisible - get a new "top" to make the cursor visible
	 * @param c the cursor
	 * @param top_y the current y location of the containing region
	 *
	 * This helper function calculates a new y co-ordinate for
	 * the top of the containing region such that the cursor contained
	 * within the LyXText is "nicely" visible.
	 */
	virtual unsigned int topCursorVisible(LyXCursor const & c, int top_y);
 
	/**
	 * fitCursor - fit the cursor onto the work area
	 * @param text the text containing the cursor
	 * @param bv the bufferview
	 * @return true if a change was necessary
	 *
	 * Scrolls the screen so that the cursor is visible,
	 */
	virtual bool fitCursor(LyXText *, BufferView *);
 
	/// show the cursor if it's not, and vice versa
	virtual void cursorToggle(BufferView *) const;

	/**
	 * update - update part of the screen rendering
	 * @param text the containing text region
	 * @param bv the bufferview
	 * @param xo the x offset into the text
	 * @param yo the x offset into the text
	 * 
	 * Updates part of the screen. If text->status is
	 * LyXText::NEED_MORE_REFRESH, we update from the
	 * point of change and to the end of the screen.
	 * If text->status is LyXText::NEED_VERY_LITTLE_REFRESH,
	 * we only update the current row. 
	 */
	virtual void update(LyXText * text, BufferView * bv, int yo = 0, int xo = 0);
 
	/// FIXME
	virtual void toggleSelection(LyXText *, BufferView *, bool = true,
			     int y_offset = 0, int x_offset = 0);
 
	/// FIXME - at least change the name !!
	virtual void toggleToggle(LyXText *, BufferView *,
			  int y_offset = 0, int x_offset = 0);
	
	/// FIXME
	virtual bool forceClear() const { return force_clear_; }

protected:
	/// cause the display of the given area of the work area
	virtual void expose(int x, int y, int w, int h) = 0;

	/// get the work area
	virtual WorkArea & workarea() const = 0;
 
	/// y1 and y2 are coordinates of the screen
	virtual void drawFromTo(LyXText *, BufferView *, int y1, int y2,
	                int y_offset = 0, int x_offset = 0, bool internal = false);

	/// y is a coordinate of the text
	virtual void drawOneRow(LyXText *, BufferView *, Row * row,
			int y_text, int y_offset = 0, int x_offset = 0);
 
	/// grey out (no buffer)
	void greyOut();
 
	/// FIXME ?
	bool force_clear_;
 
	/// is the blinking cursor currently drawn
	bool cursor_visible_;
};

#endif // SCREEN_H
