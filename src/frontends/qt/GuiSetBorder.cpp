/**
 * \file GuiSetBorder.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSetBorder.h"

#include "support/debug.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>


GuiSetBorder::GuiSetBorder(QWidget * parent, Qt::WindowFlags fl)
	: QWidget(parent, fl), buffer(75, 75), bottom_drawn_wide_(false),
	  top_drawn_wide_(false)
{
	/* length of corner line */
	corner_length = buffer.width() / 10;
	/* margin */
	margin = buffer.height() / 10;

	bwidth = buffer.width();
	bheight = buffer.height();

	init();

	setMinimumSize(bwidth, bheight);
	setMaximumSize(bwidth, bheight);
}


void GuiSetBorder::paintEvent(QPaintEvent * e)
{
	QWidget::paintEvent(e);
	QPainter painter(this);
	painter.drawPixmap(0, 0, buffer);
}


void GuiSetBorder::init()
{
	buffer.fill();
	QPainter paint(&buffer);

	paint.setPen(Qt::black);

	// Draw the corner marks
	paint.drawLine(margin + corner_length, margin,
		       margin + corner_length, margin + corner_length);
	paint.drawLine(bwidth - (margin + corner_length), margin,
		       bwidth - (margin + corner_length), margin + corner_length);

	paint.drawLine(margin, margin + corner_length,
		       margin + corner_length, margin + corner_length);
	paint.drawLine(margin, bheight - (margin + corner_length),
		       margin + corner_length, bheight - (margin + corner_length));

	paint.drawLine(margin + corner_length ,bheight - margin,
		       margin + corner_length ,bheight - (margin + corner_length));
	paint.drawLine(bwidth - (margin + corner_length), bheight - margin,
		       bwidth - (margin + corner_length), bheight - (margin + corner_length));

	paint.drawLine(bheight - margin, margin+corner_length,
		       bheight - (margin + corner_length), margin + corner_length);
	paint.drawLine(bheight - margin, bheight - (margin + corner_length),
		       bheight - (margin + corner_length),bheight - (margin + corner_length));
}


void GuiSetBorder::mousePressEvent(QMouseEvent * e)
{
	if (e->y() > e->x()) {
		if (e->y() < height() - e->x()) {
			if (left_.enabled) {
				setLeft(left_.set == LINE_SET ? LINE_UNSET : LINE_SET);
				// emit signal
				leftSet();
			}
		} else {
			if (bottom_trim_left_.enabled && e->x() < margin + 4 + 2 * corner_length) {
				setBottomLeftTrim(bottom_trim_left_.set == LINE_SET ? LINE_UNSET : LINE_SET);
				// emit signal
				bottomLTSet();
			} else if (bottom_trim_right_.enabled && e->x() > bwidth - margin - 2 * corner_length - 4) {
					setBottomRightTrim(bottom_trim_right_.set == LINE_SET ? LINE_UNSET : LINE_SET);
					// emit signal
					bottomRTSet();
			} else if (bottom_.enabled) {
				setBottom(bottom_.set == LINE_SET ? LINE_UNSET : LINE_SET);
				// emit signal
				bottomSet();
			}
		}
	} else {
		if (e->y() < height() - e->x()) {
			if (top_trim_left_.enabled && e->x() < margin + 4 + 2 * corner_length) {
				setTopLeftTrim(top_trim_left_.set == LINE_SET ? LINE_UNSET : LINE_SET);
				// emit signal
				topLTSet();
			} else if (top_trim_right_.enabled && e->x() > bwidth - margin - 2 * corner_length - 4) {
					setTopRightTrim(top_trim_right_.set == LINE_SET ? LINE_UNSET : LINE_SET);
					// emit signal
					topRTSet();
			} else if (top_.enabled) {
				setTop(top_.set == LINE_SET ? LINE_UNSET : LINE_SET);
				// emit signal
				topSet();
			}
		} else {
			if (right_.enabled) {
				setRight(right_.set == LINE_SET ? LINE_UNSET : LINE_SET);
				// emit signal
				rightSet();
			}
		}
	}
	update();
	// emit signal
	clicked();
}


void GuiSetBorder::drawLine(QColor const & col, int x, int y, int x2, int y2)
{
	QPainter paint(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(col);
	paint.setPen(p);
	paint.drawLine(x, y, x2, y2);
}


void GuiSetBorder::drawLeft(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		break;
	case LINE_UNSET:
		col = Qt::white;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		break;
	}
	if (!left_.enabled)
		col = QColor(Qt::lightGray);
	drawLine(col, margin + corner_length, margin + corner_length + 2,
		 margin + corner_length, bheight - margin - corner_length - 1);
}


void GuiSetBorder::drawRight(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		break;
	case LINE_UNSET:
		col = Qt::white;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		break;
	}
	if (!right_.enabled)
		col = QColor(Qt::lightGray);
	drawLine(col, bheight - margin - corner_length + 1, margin + corner_length + 2,
		 bheight - margin - corner_length + 1, bheight - margin - corner_length - 1);
}


void GuiSetBorder::drawTop(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		top_drawn_wide_ = true;
		break;
	case LINE_UNSET:
		col = Qt::white;
		top_drawn_wide_ = false;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		top_drawn_wide_ = true;
		break;
	}
	if (!top_.enabled)
		col = QColor(Qt::lightGray);
	int const lt = (top_trim_left_.enabled) ? corner_length + 4 : 0;
	int const rt = (top_trim_right_.enabled) ? corner_length + 4 : 0;
	drawLine(col, margin + corner_length + 2 + lt, margin + corner_length,
		 bwidth - margin - corner_length - 1 - rt, margin + corner_length);
}


void GuiSetBorder::undrawWideTopLine()
{
	if (!top_drawn_wide_)
		return;

	// Overpaint previous lines white
	drawLine(Qt::white, margin + corner_length + 2, margin + corner_length,
		 bwidth - margin - corner_length - 1, margin + corner_length);
	top_drawn_wide_ = false;
}


void GuiSetBorder::drawTopLeftTrim(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		break;
	case LINE_UNSET:
		col = Qt::white;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		break;
	}
	if (!top_trim_left_.enabled)
		col = QColor(Qt::white);
	int const lt = corner_length;
	if (top_trim_left_.enabled)
		drawLine(col, margin + corner_length + 2, margin + corner_length,
			 margin + corner_length + 2 + lt, margin + corner_length);
}


void GuiSetBorder::drawTopRightTrim(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		break;
	case LINE_UNSET:
		col = Qt::white;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		break;
	}
	if (!top_trim_right_.enabled)
		col = QColor(Qt::white);
	int const rt = corner_length;
	if (top_trim_right_.enabled)
		drawLine(col, bwidth - margin - corner_length - 1 - rt, margin + corner_length,
			bwidth - margin - corner_length - 1, margin + corner_length);
}


void GuiSetBorder::drawBottom(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		bottom_drawn_wide_ = true;
		break;
	case LINE_UNSET:
		col = Qt::white;
		bottom_drawn_wide_ = false;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		bottom_drawn_wide_ = true;
		break;
	}
	if (!bottom_.enabled)
		col = QColor(Qt::lightGray);
	int const lt = (bottom_trim_left_.enabled) ? corner_length + 4 : 0;
	int const rt = (bottom_trim_right_.enabled) ? corner_length + 4 : 0;
	drawLine(col, margin + corner_length + 2 + lt, bwidth - margin - corner_length + 1,
		 bwidth - margin - corner_length - 1 - rt, bwidth - margin - corner_length + 1);
}


void GuiSetBorder::undrawWideBottomLine()
{
	if (!bottom_drawn_wide_)
		return;

	//Overpaint previous lines white
	drawLine(Qt::white, margin + corner_length + 2, bwidth - margin - corner_length + 1,
		 bwidth - margin - corner_length - 1, bwidth - margin - corner_length + 1);
	bottom_drawn_wide_ = false;
}


void GuiSetBorder::drawBottomLeftTrim(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		break;
	case LINE_UNSET:
		col = Qt::white;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		break;
	}
	if (!bottom_trim_left_.enabled)
		col = QColor(Qt::white);
	int const lt = corner_length;
	if (bottom_trim_left_.enabled)
		drawLine(col, margin + corner_length + 2, bwidth - margin - corner_length + 1,
			 margin + corner_length + 2 + lt, bwidth - margin - corner_length + 1);
}


void GuiSetBorder::drawBottomRightTrim(BorderState draw)
{
	QColor col;
	switch (draw) {
	case LINE_SET:
		col = Qt::black;
		break;
	case LINE_UNSET:
		col = Qt::white;
		break;
	case LINE_UNDECIDED:
	case LINE_UNDEF:
		col = Qt::lightGray;
		break;
	}
	if (!bottom_trim_right_.enabled)
		col = QColor(Qt::white);
	int const rt = corner_length;
	if (bottom_trim_right_.enabled)
		drawLine(col, bwidth - margin - corner_length - 1 - rt, bwidth - margin - corner_length + 1,
			 bwidth - margin - corner_length - 1, bwidth - margin - corner_length + 1);
}


void GuiSetBorder::setLeftEnabled(bool enabled)
{
	left_.enabled = enabled;
	drawLeft(left_.set);
}


void GuiSetBorder::setRightEnabled(bool enabled)
{
	right_.enabled = enabled;
	drawRight(right_.set);
}


void GuiSetBorder::setTopEnabled(bool enabled)
{
	top_.enabled = enabled;
	drawTop(top_.set);
}


void GuiSetBorder::setBottomEnabled(bool enabled)
{
	bottom_.enabled = enabled;
	drawBottom(bottom_.set);
}


void GuiSetBorder::setTopLeftTrimEnabled(bool enabled)
{
	top_trim_left_.enabled = enabled;
	undrawWideTopLine();
	drawTopLeftTrim(top_trim_left_.set);
	drawTop(top_.set);
	top_drawn_wide_ = !enabled;
}


void GuiSetBorder::setTopRightTrimEnabled(bool enabled)
{
	top_trim_right_.enabled = enabled;
	undrawWideTopLine();
	drawTopRightTrim(top_trim_right_.set);
	drawTop(top_.set);
	top_drawn_wide_ = !enabled;;
}


void GuiSetBorder::setBottomLeftTrimEnabled(bool enabled)
{
	bottom_trim_left_.enabled = enabled;
	undrawWideBottomLine();
	drawBottomLeftTrim(bottom_trim_left_.set);
	drawBottom(bottom_.set);
	bottom_drawn_wide_ = !enabled;;
}


void GuiSetBorder::setBottomRightTrimEnabled(bool enabled)
{
	bottom_trim_right_.enabled = enabled;
	undrawWideBottomLine();
	drawBottomRightTrim(bottom_trim_right_.set);
	drawBottom(bottom_.set);
	bottom_drawn_wide_ = !enabled;;
}


void GuiSetBorder::setLeft(BorderState border)
{
	left_.set = border;
	drawLeft(border);
}


void GuiSetBorder::setRight(BorderState border)
{
	right_.set = border;
	drawRight(border);
}


void GuiSetBorder::setTop(BorderState border)
{
	top_.set = border;
	drawTop(border);
}


void GuiSetBorder::setBottom(BorderState border)
{
	bottom_.set = border;
	drawBottom(border);
}


void GuiSetBorder::setTopLeftTrim(BorderState border)
{
	top_trim_left_.set = border;
	drawTopLeftTrim(border);
}


void GuiSetBorder::setTopRightTrim(BorderState border)
{
	top_trim_right_.set = border;
	drawTopRightTrim(border);
}


void GuiSetBorder::setBottomLeftTrim(BorderState border)
{
	bottom_trim_left_.set = border;
	drawBottomLeftTrim(border);
}

void GuiSetBorder::setBottomRightTrim(BorderState border)
{
	bottom_trim_right_.set = border;
	drawBottomRightTrim(border);
}


void GuiSetBorder::setAll(BorderState border)
{
	setLeft(border);
	setRight(border);
	setTop(border);
	setBottom(border);
	setTopLeftTrim(border);
	setTopRightTrim(border);
	setBottomLeftTrim(border);
	setBottomRightTrim(border);
}


GuiSetBorder::BorderState GuiSetBorder::getLeft()
{
	return left_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getRight()
{
	return right_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getTop()
{
	return top_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getBottom()
{
	return bottom_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getTopLeftTrim()
{
	return top_trim_left_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getTopRightTrim()
{
	return top_trim_right_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getBottomLeftTrim()
{
	return bottom_trim_left_.set;
}


GuiSetBorder::BorderState GuiSetBorder::getBottomRightTrim()
{
	return bottom_trim_right_.set;
}

#include "moc_GuiSetBorder.cpp"
