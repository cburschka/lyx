// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team
 *
 * ====================================================== */

#ifndef LYXSCREEN_H
#define LYXSCREEN_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include <X11/Xlib.h>

class LyXText;
struct Row;
typedef unsigned short Dimension;

class WorkArea;

/** The class LyXScreen is used for the main Textbody.
    Concretely, the screen is held in a pixmap.  This pixmap is kept up to
    date and used to optimize drawing on the screen.
    This class also handles the drawing of the cursor and partly the selection.
 */
class LyXScreen {
public:

	enum Cursor_Shape {
		///
		BAR_SHAPE,
		///
		L_SHAPE,
		///
		REVERSED_L_SHAPE
	};

	///
	LyXScreen(WorkArea &, LyXText * text_ptr);

	/** Draws the screen form textposition y. Uses as much of
	    the already printed pixmap as possible */
	void Draw(unsigned long y );

	/// Redraws the screen, without using existing pixmap
	void Redraw();
   
	/// Returns a new top so that the cursor is visible
	unsigned long TopCursorVisible();
	/// Redraws the screen such that the cursor is visible
	bool FitCursor();
	///
	void ShowCursor();
	///
	void HideCursor();
	///
	void CursorToggle();
	///
	void ShowManualCursor(long x, long y, int asc, int desc,
			      Cursor_Shape shape);
	/// returns 1 if first has changed, otherwise 0
	bool FitManualCursor(long, long, int, int);
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

#if 0
	/** Updates part of the screen. Updates till row with cursor,
	    or only current row */
	void SmallUpdate();
#endif
	/// first visible pixel-row
	unsigned long first;

	///
	bool cursor_visible;
private:
	/// Copies specified area of pixmap to screen
	void expose(int x, int y, int exp_width, int exp_height); 

	/// y1 and y2 are coordinates of the screen
	void DrawFromTo(int y1, int y2);

#if 1
	/// y is a coordinate of the text
	void DrawOneRow(Row * row, long y_text);
#else
	/// y is a coordinate of the text
	void DrawOneRow(Row * row, long & y_text);
#endif

	///
	WorkArea & owner;
	
	///
	LyXText * text;

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
};

#endif
