// -*- C++ -*-
/**
 * \file xscreen.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XSCREEN_H
#define XSCREEN_H


#include "screen.h"
#include <X11/Xlib.h> // for Pixmap, GC

class WorkArea;
class XWorkArea;


/** The class XScreen is used for the main Textbody.
    Concretely, the screen is held in a pixmap.  This pixmap is kept up to
    date and used to optimize drawing on the screen.
    This class also handles the drawing of the cursor and partly the selection.
 */
class XScreen : public LyXScreen {
public:
	///
	XScreen(XWorkArea &);

	///
	virtual ~XScreen();

	/// Sets the cursor color to LColor::cursor.
	virtual	void setCursorColor();

protected:
	/// get the work area
	virtual WorkArea & workarea() const;

	/// Copies specified area of pixmap to screen
	virtual void expose(int x, int y, int w, int h);

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, Cursor_Shape shape);

	/// hide the cursor
	virtual void removeCursor();

private:
	/// our owning widget
	XWorkArea & owner_;

	/// backing pixmap for cursor
	Pixmap nocursor_pixmap_;
	/// x of backing pixmap
	int cursor_x_;
	/// y of backing pixmap
	int cursor_y_;
	/// width of backing pixmap
	int cursor_w_;
	/// height of backing pixmap
	int cursor_h_;
	/// cursor cs
	GC gc_copy;
};

#endif
