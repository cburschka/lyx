// -*- C++ -*-
/**
 * \file GScreen.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GSCREEN_H
#define GSCREEN_H

#include "screen.h"

namespace lyx {
namespace frontend {

class GWorkArea;

/** The class GScreen is used for the main Textbody.
    Concretely, the screen is held in a pixmap.  This pixmap is kept up to
    date and used to optimize drawing on the screen.
    This class also handles the drawing of the cursor and partly the selection.
 */
class GScreen : public LyXScreen {
public:
	///
	GScreen(GWorkArea &);

	///
	virtual ~GScreen();

	/// Sets the cursor color to LColor::cursor.
	virtual	void setCursorColor(Glib::RefPtr<Gdk::GC> gc);
	///
	virtual void removeCursor();
	///
	virtual void showCursor(int x, int y, int h, Cursor_Shape shape);
protected:
	/// get the work area
	virtual WorkArea & workarea() const;

	/// Copies specified area of pixmap to screen
	virtual void expose(int x, int y, int w, int h);
private:
	/// our owning widget
	GWorkArea & owner_;

	///
	Glib::RefPtr<Gdk::Pixmap> cursorPixmap_;
	///
	int cursorX_;
	///
	int cursorY_;
	///
	int cursorW_;
	///
	int cursorH_;
};

} // namespace frontend
} // namespace lyx

#endif
