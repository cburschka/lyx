// -*- C++ -*-
/**
 * \file screen.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "RowList.h"

class LyXText;
class LyXCursor;
class WorkArea;
class BufferView;

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
	 * Scrolls the screen so that the cursor is visible
	 */
	virtual bool fitCursor(LyXText *, BufferView *);

	/**
	 * update - update part of the screen rendering
	 * @param bv the bufferview
	 * @param xo the x offset into the text
	 * @param yo the x offset into the text
	 *
	 * Updates part of the screen. If bv->text->status is
	 * LyXText::REFRESH_AREA, we update from the
	 * point of change and to the end of the screen.
	 * If text->status is LyXText::REFRESH_ROW,
	 * we only update the current row.
	 */
	virtual void update(BufferView & bv, int yo = 0, int xo = 0);

	/// hide the visible cursor, if it is visible
	void hideCursor();
	
	/// show the cursor if it is not visible
	void showCursor(BufferView & bv);

	/// toggle the cursor's visibility
	void toggleCursor(BufferView & bv);

	/// FIXME
	virtual void toggleSelection(LyXText *, BufferView *, bool = true,
			     int y_offset = 0, int x_offset = 0);

	/// FIXME - at least change the name !!
	virtual void toggleToggle(LyXText *, BufferView *,
			  int y_offset = 0, int x_offset = 0);

protected:
	/// cause the display of the given area of the work area
	virtual void expose(int x, int y, int w, int h) = 0;

	/// get the work area
	virtual WorkArea & workarea() const = 0;

	/// types of cursor in work area
	enum Cursor_Shape {
		/// normal I-beam
		BAR_SHAPE,
		/// L-shape for locked insets of a different language
		L_SHAPE,
		/// reverse L-shape for RTL text
		REVERSED_L_SHAPE
	};

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, Cursor_Shape shape) = 0;

	/// hide the cursor
	virtual void removeCursor() = 0;

	/// y1 and y2 are coordinates of the screen
	void drawFromTo(LyXText *, BufferView *, int y1, int y2,
	                int y_offset = 0, int x_offset = 0);

	/// y is a coordinate of the text
	void drawOneRow(LyXText *, BufferView *,
			RowList::iterator row,
			int y_text, int y_offset = 0, int x_offset = 0);

private:
	/// grey out (no buffer)
	void greyOut();

	/// is the cursor currently displayed
	bool cursor_visible_;

	/// is the screen displaying text or the splash screen?
	bool greyed_out_;
};

#endif // SCREEN_H
