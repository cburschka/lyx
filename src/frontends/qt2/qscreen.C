/**
 * \file qscreen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>
#include <iostream>

#include "LColor.h"
#include "QWorkArea.h"
#include "qscreen.h"
#include "lyxtext.h"
#include "Painter.h"
#include "BufferView.h"
#include "insets/insettext.h"
#include "debug.h"

#include <qapplication.h>

using std::endl;
using std::max;
using std::min;

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


void QScreen::showManualCursor(LyXText const * text, int x, int y,
				 int asc, int desc, Cursor_Shape shape)
{
	if (!qApp->focusWidget())
		return;

	string const focusname(qApp->focusWidget()->name());

	// Probably a hack
	if (focusname != "content_pane")
		return;

	int const y1 = max(y - text->first_y - asc, 0);
	int const y_tmp = min(y - text->first_y + desc, owner_.height());

	// secure against very strange situations
	// which would be when .... ?
	int const y2 = max(y_tmp, y1);

	if (y2 > 0 && y1 < owner_.height()) {
		cursor_h_ = y2 - y1 + 1;
		cursor_y_ = y1;

		switch (shape) {
		case BAR_SHAPE:
			cursor_w_ = 1;
			cursor_x_ = x;
			break;
		case L_SHAPE:
			cursor_w_ = cursor_h_ / 3;
			cursor_x_ = x;
			break;
		case REVERSED_L_SHAPE:
			cursor_w_ = cursor_h_ / 3;
			cursor_x_ = x - cursor_w_ + 1;
			break;
		}

		if (!nocursor_pixmap_.get()
			|| cursor_w_ != nocursor_pixmap_->width()
			|| cursor_h_ != nocursor_pixmap_->height()) {
			nocursor_pixmap_.reset(new QPixmap(cursor_w_, cursor_h_));
		}

		owner_.getPainter().start();

		// save old area
		bitBlt(nocursor_pixmap_.get(), 0, 0, owner_.getPixmap(),
			cursor_x_, cursor_y_, cursor_w_, cursor_h_);

		owner_.getPainter().line(x, y1, x, y2);
		switch (shape) {
		case BAR_SHAPE:
			break;
		case L_SHAPE:
		case REVERSED_L_SHAPE:
			int const rectangle_h = (cursor_h_ + 10) / 20;
			owner_.getPainter().fillRectangle(
				cursor_x_, y2 - rectangle_h + 1,
				cursor_w_ - 1, rectangle_h, LColor::cursor);
			break;
		}

		owner_.getPainter().end();

		owner_.getContent()->repaint(
			cursor_x_, cursor_y_,
			cursor_w_, cursor_h_);

	}
	cursor_visible_ = true;
}


void QScreen::hideCursor()
{
	if (!cursor_visible_)
		return;

	bitBlt(owner_.getPixmap(), cursor_x_, cursor_y_,
		nocursor_pixmap_.get(), 0, 0, cursor_w_, cursor_h_);

	owner_.getContent()->repaint(
		cursor_x_, cursor_y_,
		cursor_w_, cursor_h_);

	cursor_visible_ = false;
}


void QScreen::repaint()
{
	QWidget * content(owner_.getContent());
	content->repaint(0, 0, content->width(), content->height());
}


void QScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "expose " << w << "x" << h
		<< "+" << x << "+" << y << endl;

	owner_.getContent()->update(x, y, w, h);
}


void QScreen::draw(LyXText * text, BufferView * bv, unsigned int y)
{
	QPixmap * p(owner_.getPixmap());

	owner_.getPainter().start();

	if (cursor_visible_) hideCursor();

	int const old_first = text->first_y;
	bool const internal = (text == bv->text);
	text->first_y = y;

	// If you want to fix the warning below, fix it so it
	// actually scrolls properly. Hint: a cast won't do.

	// is any optimization possible?
	if (y - old_first < owner_.workHeight()
	    && old_first - y < owner_.workHeight()) {
		if (text->first_y < old_first) {
			int const dest_y = old_first - text->first_y;
			drawFromTo(text, bv, 0, dest_y, 0, 0, internal);
			copyInPixmap(p, dest_y, 0, owner_.workWidth(), owner_.height() - dest_y);
			expose(0, 0, owner_.workWidth(), dest_y);
		} else  {
			int const src_y = text->first_y - old_first;
			drawFromTo(text, bv, owner_.height() - src_y, owner_.height(), 0, 0, internal);
			copyInPixmap(p, 0, 0, owner_.workWidth(), owner_.height() - src_y);
			expose(0, owner_.height() - src_y, owner_.workWidth(), src_y);
		}
	} else {
		lyxerr[Debug::GUI] << "dumb full redraw" << endl;
		drawFromTo(text, bv, 0, owner_.height(), 0, 0, internal);
		repaint();
	}

	owner_.getPainter().end();
}
