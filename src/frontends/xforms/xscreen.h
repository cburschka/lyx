// -*- C++ -*-
/**
 * \file xscreen.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef XSCREEN_H
#define XSCREEN_H

#ifdef __GNUG__
#pragma interface
#endif

#include <X11/Xlib.h>

#include "screen.h"

#include "XWorkArea.h"
 
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
	///
	virtual void hideCursor();
	///
	virtual void showManualCursor(LyXText const *, int x, int y,
			      int asc, int desc,
			      Cursor_Shape shape);
	
	/** Draws the screen form textposition y. Uses as much of
	    the already printed pixmap as possible */
	virtual void draw(LyXText *, BufferView *, unsigned int y);

protected:
	/// get the work area
	virtual WorkArea & workarea() const { return owner_; }
 
	/// Copies specified area of pixmap to screen
	virtual void expose(int x, int y, int exp_width, int exp_height); 
 
private:
	/// our owning widget
	XWorkArea & owner_;
 
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
