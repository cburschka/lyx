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

class BufferView;

/** The class LyXScreen is used for the main Textbody.
    Concretely, the screen is held in a pixmap.  This pixmap is kept up to
    date and used to optimize drawing on the screen.
    This class also handles the drawing of the cursor and partly the selection.
 */
class LyXScreen {
public:
	///
	LyXScreen(BufferView *, Window window,
		  Pixmap p,
		  Dimension width, 
		  Dimension height,
		  Dimension offset_x,
		  Dimension offset_y,
		  LyXText * text_ptr);
	///
	~LyXScreen();

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
	void HideManualCursor();
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
	void DrawOneRow(Row * row, long & y_text);

	///
	BufferView * owner;
	
	///
	LyXText * text;

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
	Row * screen_refresh_row;

	///
	GC gc_copy;
};

#endif
