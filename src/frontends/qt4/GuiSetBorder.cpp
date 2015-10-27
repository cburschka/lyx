/**
 * \file GuiSetBorder.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSetBorder.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>


GuiSetBorder::GuiSetBorder(QWidget * parent, Qt::WindowFlags fl)
	: QWidget(parent, fl), buffer(75, 75)
{
	/* length of corner line */
	l = buffer.width() / 10;
	/* margin */
	m = buffer.height() / 10;

	w = buffer.width();
	h = buffer.height();

	init();

	setMinimumSize(w,h);
	setMaximumSize(w,h);
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

	// FIXME: wow, readable !! :)

	paint.drawLine(m + l , m, m + l, m + l);
	paint.drawLine(w - (m + l), m, w - (m + l), m + l);

	paint.drawLine(m, m + l , m + l, m + l);
	paint.drawLine(m, h - (m + l), m + l, h - (m + l));

	paint.drawLine(m + l ,h - m, m + l ,h - (m + l));
	paint.drawLine(w - (m + l), h - m, w - (m + l), h - (m + l));

	paint.drawLine(h - m, m+l, h - (m + l), m + l);
	paint.drawLine(h - m, h - (m + l), h - (m + l),h - (m + l));
}


void GuiSetBorder::mousePressEvent(QMouseEvent * e)
{
	if (e->y() > e->x()) {
		if (e->y() < height() - e->x()) {
			if (left_.enabled) {
				setLeft(!left_.set);
				// emit signal
				leftSet(left_.set);
			}
		} else {
			if (bottom_.enabled) {
				setBottom(!bottom_.set);
				// emit signal
				bottomSet(bottom_.set);
			}
		}
	} else {
		if (e->y() < height() - e->x()) {
			if (top_.enabled) {
				setTop(!top_.set);
				// emit signal
				topSet(top_.set);
			}
		} else {
			if (right_.enabled) {
				setRight(!right_.set);
				// emit signal
				rightSet(right_.set);
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


void GuiSetBorder::drawLeft(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!left_.enabled)
		col = QColor(Qt::lightGray);
	drawLine(col, m + l, m + l + 2, m + l, h - m - l - 1);
}


void GuiSetBorder::drawRight(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!right_.enabled)
		col = QColor(Qt::lightGray);
	drawLine(col, h - m - l + 1, m + l + 2, h - m - l + 1, h - m - l - 1);
}


void GuiSetBorder::drawTop(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!top_.enabled)
		col = QColor(Qt::lightGray);
	drawLine(col, m + l + 2, m + l, w - m - l - 1, m + l);
}


void GuiSetBorder::drawBottom(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!bottom_.enabled)
		col = QColor(Qt::lightGray);
	drawLine(col, m + l + 2, w - m - l + 1, w - m - l - 1, w - m - l + 1);
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


void GuiSetBorder::setLeft(bool border)
{
	left_.set = border;
	drawLeft(border);
}


void GuiSetBorder::setRight(bool border)
{
	right_.set = border;
	drawRight(border);
}


void GuiSetBorder::setTop(bool border)
{
	top_.set = border;
	drawTop(border);
}


void GuiSetBorder::setBottom(bool border)
{
	bottom_.set = border;
	drawBottom(border);
}


void GuiSetBorder::setAll(bool border)
{
	setLeft(border);
	setRight(border);
	setTop(border);
	setBottom(border);
}


bool GuiSetBorder::getLeft()
{
	return left_.set;
}


bool GuiSetBorder::getRight()
{
	return right_.set;
}


bool GuiSetBorder::getTop()
{
	return top_.set;
}


bool GuiSetBorder::getBottom()
{
	return bottom_.set;
}

#include "moc_GuiSetBorder.cpp"
