/**
 * \file qscreen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QWorkArea.h"
#include "qscreen.h"

#include "debug.h"
#include "lcolorcache.h"

#include <qapplication.h>


namespace {

/// copy some horizontal regions about inside a pixmap
void copyInPixmap(QPixmap * p, int dest_y, int src_y, int src_w, int src_h)
{
	// bitBlt(dest, dest_x, dest_y, source, src_x, src_y, src_w, src_h)
	bitBlt(p, 0, dest_y, p, 0, src_y, src_w, src_h);
}

} // namespace anon


QScreen::QScreen(QWorkArea & o)
	: LyXScreen(), owner_(o)
{
}


QScreen::~QScreen()
{
}


WorkArea & QScreen::workarea() const
{
	return owner_;
}


void QScreen::repaint()
{
	QWidget * content = owner_.getContent();
	content->repaint(0, 0, content->width(), content->height());
}


void QScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "expose " << w << 'x' << h
		<< '+' << x << '+' << y << std::endl;

	owner_.getContent()->update(x, y, w, h);
}


void QScreen::showCursor(int x, int y, int h, Cursor_Shape shape)
{
	if (!qApp->focusWidget())
		return;

	// Cache the dimensions of the cursor.
	cursor_x_ = x;
	cursor_y_ = y;
	cursor_h_ = h;

	switch (shape) {
	case BAR_SHAPE:
		cursor_w_ = 1;
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

	// Initialise storage for these pixmaps as necessary.
	if (cursor_w_ != nocursor_pixmap_.width() ||
	    cursor_h_ != nocursor_pixmap_.height()) {
		nocursor_pixmap_.resize(cursor_w_, cursor_h_);
	}

	QColor const & required_color = lcolorcache.get(LColor::cursor);
	bool const cursor_color_changed = required_color != cursor_color_;
	if (cursor_color_changed)
		cursor_color_ = required_color;

	if (cursor_h_ != vcursor_pixmap_.height() || cursor_color_changed) {
		if (cursor_h_ != vcursor_pixmap_.height())
			vcursor_pixmap_.resize(1, cursor_h_);
		vcursor_pixmap_.fill(cursor_color_);
	}

	switch (shape) {
	case BAR_SHAPE:
		break;
	case REVERSED_L_SHAPE:
	case L_SHAPE:
		if (cursor_w_ != hcursor_pixmap_.width() ||
		    cursor_color_changed) {
			if (cursor_w_ != hcursor_pixmap_.width())
				hcursor_pixmap_.resize(cursor_w_, 1);
			hcursor_pixmap_.fill(cursor_color_);
		}
		break;
	}

	// Save the old area (no cursor).
	bitBlt(&nocursor_pixmap_, 0, 0, owner_.getPixmap(),
	       cursor_x_, cursor_y_, cursor_w_, cursor_h_);

	// Draw the new (vertical) cursor using the cached store.
	bitBlt(owner_.getPixmap(), x, y,
	       &vcursor_pixmap_, 0, 0,
	       vcursor_pixmap_.width(), vcursor_pixmap_.height());

	// Draw the new (horizontal) cursor if necessary.
	switch (shape) {
	case BAR_SHAPE:
		break;
	case REVERSED_L_SHAPE:
	case L_SHAPE:
		bitBlt(owner_.getPixmap(), cursor_x_, y + h - 1,
		       &hcursor_pixmap_, 0, 0,
		       hcursor_pixmap_.width(), hcursor_pixmap_.height());
		break;
	}

	owner_.getContent()->repaint(
		cursor_x_, cursor_y_,
		cursor_w_, cursor_h_);
}


void QScreen::removeCursor()
{
	// before first showCursor
	if (nocursor_pixmap_.isNull())
		return;

	bitBlt(owner_.getPixmap(), cursor_x_, cursor_y_,
	       &nocursor_pixmap_, 0, 0, cursor_w_, cursor_h_);

	owner_.getContent()
		->repaint(cursor_x_, cursor_y_, cursor_w_, cursor_h_);
}
