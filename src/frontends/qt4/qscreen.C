/**
 * \file qscreen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QWorkArea.h"
#include "qscreen.h"

#include <QColor>
#include <QPainter>
#include <QApplication>

#include "debug.h"
#include "lcolorcache.h"


namespace {

} // namespace anon


QScreen::QScreen(QWorkArea & o)
	: LyXScreen(), owner_(o), nocursor_(0,0)
{
}


QScreen::~QScreen()
{
}


WorkArea & QScreen::workarea() const
{
	return owner_;
}

void QScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "expose " << w << 'x' << h
		<< '+' << x << '+' << y << std::endl;

	owner_.update(x, y, w, h);
}

void QScreen::showCursor(int x, int y, int h, Cursor_Shape shape)
{
	if (!qApp->focusWidget())
		return;

	if (x==cursor_x_ && y==cursor_y_ && h==cursor_h_) {
		// Draw the new (vertical) cursor using the cached store.
		owner_.drawScreen(cursor_x_, cursor_y_, vcursor_);
		return;
	}
		
	// Cache the dimensions of the cursor.
	cursor_x_ = x;
	cursor_y_ = y;
	cursor_h_ = h;

	switch (shape) {
	case BAR_SHAPE:
		cursor_w_ = 2;
		break;
	case L_SHAPE:
		cursor_w_ = cursor_h_ / 3;
		break;
	case REVERSED_L_SHAPE:
		cursor_w_ = cursor_h_ / 3;
		cursor_x_ = x - cursor_w_ + 1;
		break;
	}


	// We cache three pixmaps:
	// 1 the rectangle of the original screen.
	// 2 the vertical line of the cursor.
	// 3 the horizontal line of the L-shaped cursor (if necessary).

	QColor const & required_color = lcolorcache.get(LColor::cursor);
	bool const cursor_color_changed = required_color != cursor_color_;
	if (cursor_color_changed)
		cursor_color_ = required_color;

	vcursor_ = QPixmap(cursor_w_, cursor_h_);
	vcursor_ .fill(cursor_color_);

	switch (shape) {
	case BAR_SHAPE:
		break;
	case REVERSED_L_SHAPE:
	case L_SHAPE:
		if (cursor_w_ != hcursor_.width() ||
		    cursor_color_changed) {
			if (cursor_w_ != hcursor_.width())
				hcursor_ = QPixmap(cursor_w_, 1);
			hcursor_.fill(cursor_color_);
		}
		break;
	}

	// Save the old area (no cursor).
	nocursor_ = owner_.copyScreen(cursor_x_, cursor_y_, cursor_w_, cursor_h_);

	// Draw the new (vertical) cursor using the cached store.
	owner_.drawScreen(cursor_x_, cursor_y_, vcursor_);

	// Draw the new (horizontal) cursor if necessary.
	switch (shape) {
	case BAR_SHAPE:
		break;
	case REVERSED_L_SHAPE:
	case L_SHAPE:
		owner_.drawScreen(cursor_x_, y + h - 1, hcursor_);
		break;
	}
}


void QScreen::removeCursor()
{
	// before first showCursor
	if (nocursor_.isNull())
		return;

	owner_.drawScreen(cursor_x_, cursor_y_, nocursor_);
}
