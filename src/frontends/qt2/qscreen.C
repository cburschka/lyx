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


void QScreen::repaint()
{
	QWidget * content(owner_.getContent());
	content->repaint(0, 0, content->width(), content->height());
}


void QScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "expose " << w << 'x' << h
		<< '+' << x << '+' << y << endl;

	owner_.getContent()->update(x, y, w, h);
}


void QScreen::draw(LyXText * text, BufferView * bv, unsigned int y)
{
	QPixmap * p(owner_.getPixmap());

	owner_.getPainter().start();

	int const old_first = text->top_y();
	text->top_y(y);

	// If you want to fix the warning below, fix it so it
	// actually scrolls properly. Hint: a cast won't do.

	// is any optimization possible?
	if (y - old_first < owner_.workHeight()
	    && old_first - y < owner_.workHeight()) {
		if (text->top_y() < old_first) {
			int const dest_y = old_first - text->top_y();
			drawFromTo(text, bv, 0, dest_y, 0, 0);
			copyInPixmap(p, dest_y, 0, owner_.workWidth(), owner_.height() - dest_y);
			expose(0, 0, owner_.workWidth(), dest_y);
		} else  {
			int const src_y = text->top_y() - old_first;
			drawFromTo(text, bv, owner_.height() - src_y, owner_.height(), 0, 0);
			copyInPixmap(p, 0, 0, owner_.workWidth(), owner_.height() - src_y);
			expose(0, owner_.height() - src_y, owner_.workWidth(), src_y);
		}
	} else {
		lyxerr[Debug::GUI] << "dumb full redraw" << endl;
		drawFromTo(text, bv, 0, owner_.height(), 0, 0);
		repaint();
	}

	owner_.getPainter().end();
}


void QScreen::showCursor(int x, int y, int h, Cursor_Shape shape)
{
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

	if (!nocursor_pixmap_.get()
		|| cursor_w_ != nocursor_pixmap_->width()
		|| cursor_h_ != nocursor_pixmap_->height()) {
		nocursor_pixmap_.reset(new QPixmap(cursor_w_, cursor_h_));
	}

	// save old area
	bitBlt(nocursor_pixmap_.get(), 0, 0, owner_.getPixmap(),
		cursor_x_, cursor_y_, cursor_w_, cursor_h_);

	if (!qApp->focusWidget())
		return;

	// We could have pending paints. If we don't process them
	// before, we end up with pixel dirt when the cursor is
	// in insets
	qApp->processEvents();

	Painter & pain(owner_.getPainter());
	pain.start();
	pain.line(x, y, x, y + h - 1, LColor::cursor);

	switch (shape) {
		case BAR_SHAPE:
			break;
		case REVERSED_L_SHAPE:
		case L_SHAPE:
			pain.line(cursor_x_, y + h - 1, cursor_x_ + cursor_w_ - 1,
				y + h - 1, LColor::cursor);
			break;
	}

	pain.end();

	owner_.getContent()->repaint(
		cursor_x_, cursor_y_,
		cursor_w_, cursor_h_);
}


void QScreen::removeCursor()
{
	// before first showCursor
	if (!nocursor_pixmap_.get())
		return;

	bitBlt(owner_.getPixmap(), cursor_x_, cursor_y_,
		nocursor_pixmap_.get(), 0, 0, cursor_w_, cursor_h_);

	owner_.getContent()->repaint(
		cursor_x_, cursor_y_,
		cursor_w_, cursor_h_);
}
