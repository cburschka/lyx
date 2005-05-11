// -*- C++ -*-
/**
 * \file screen.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SCREEN_H
#define SCREEN_H


class LyXText;
class CursorSlice;
class WorkArea;
class BufferView;
class ViewMetricsInfo;


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

	/// redraw the screen, without using existing pixmap
	virtual void redraw(BufferView & bv, ViewMetricsInfo const & vi);

	/// grey out (no buffer)
	void greyOut();

	/// hide the visible cursor, if it is visible
	void hideCursor();

	/// show the cursor if it is not visible
	void showCursor(BufferView & bv);

	/// toggle the cursor's visibility
	void toggleCursor(BufferView & bv);

	///
	void unAllowSync() { sync_allowed_ = false; };

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

private:
	///
	void checkAndGreyOut();

	///
	bool greyed_out_;

	/// is the cursor currently displayed
	bool cursor_visible_;

	///
	bool sync_allowed_;
};

#endif // SCREEN_H
