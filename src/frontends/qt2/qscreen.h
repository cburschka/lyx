// -*- C++ -*-
/**
 * \file qscreen.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSCREEN_H
#define QSCREEN_H

#ifdef __GNUG__
#pragma interface
#endif

#include "screen.h"
#include <qrect.h>
#include <boost/scoped_ptr.hpp>

class WorkArea;


/**
 * Qt implementation of toolkit-specific parts of LyXScreen.
 */
class QScreen : public LyXScreen {
public:
	QScreen(QWorkArea &);

	virtual ~QScreen();

	/**
	 * draw the screen from a given position
	 * @param y the text position to draw from
	 *
	 * Uses as much of the already printed pixmap as possible
	 */
	virtual void draw(LyXText *, BufferView *, unsigned int y);

	/**
	 * showManualCursor - display the caret on the work area
	 * @param text the lyx text containing the cursor
	 * @param x the x position of the cursor
	 * @param y the y position of the row's baseline
	 * @param asc ascent of the row
	 * @param desc descent of the row
	 * @param shape the current shape
	 */
	virtual void showManualCursor(LyXText const *, int x, int y,
			      int asc, int desc,
			      Cursor_Shape shape);

	/// unpaint the cursor painted by showManualCursor()
	virtual void hideCursor();

protected:
	/// get the work area
	virtual WorkArea & workarea() const { return owner_; }

	/// repaint the whole content immediately
	void repaint();

	/// copies specified area of pixmap to screen
	virtual void expose(int x, int y, int exp_width, int exp_height);

private:
	/// our owning widget
	QWorkArea & owner_;

	/// the mini-pixmap used for backing store for the blinking cursor
	boost::scoped_ptr<QPixmap> nocursor_pixmap_;

	//{@ the cursor pixmap position/size
	int cursor_x_;
	int cursor_y_;
	int cursor_w_;
	int cursor_h_;
	//@}
};

#endif // QSCREEN_H
