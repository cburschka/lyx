// -*- C++ -*-
/**
 * \file qscreen.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSCREEN_H
#define QSCREEN_H

#include "frontends/GuiCursor.h"

#include <qcolor.h>
#include <qpixmap.h>

namespace lyx {
namespace frontend {

class QWorkArea;


/**
 * Qt implementation of toolkit-specific parts of LyXScreen.
 */
class QScreen {
public:
	QScreen(QWorkArea &);

	virtual ~QScreen();

public:
	/// repaint the whole content immediately
	void repaint();

	/// copies specified area of pixmap to screen
	virtual void expose(int x, int y, int exp_width, int exp_height);

	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, lyx::frontend::CursorShape shape);

	/// hide the cursor
	virtual void removeCursor();

private:
	/// our owning widget
	QWorkArea & owner_;

	QPixmap nocursor_pixmap_;
	QPixmap hcursor_pixmap_;
	QPixmap vcursor_pixmap_;

	//@{ the cursor pixmap position/size
	int cursor_x_;
	int cursor_y_;
	int cursor_w_;
	int cursor_h_;
	//@}

	QColor cursor_color_;
};

} // namespace frontend
} // namespace lyx

#endif // QSCREEN_H
