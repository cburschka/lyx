/**
 * \file QSetBorder.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include "qsetborder.h"


QSetBorder::QSetBorder(QWidget * parent, char const * name, WFlags fl)
	: QWidget(parent, name, fl), buffer(75, 75)
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
 

void QSetBorder::paintEvent(QPaintEvent * e)
{
	QWidget::paintEvent(e);
	bitBlt(this, 0, 0, &buffer, 0, 0, width(), height());
}


void QSetBorder::init()
{
	buffer.fill();
	QPainter paint;
	paint.begin(&buffer);
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

	paint.end();
}

 
void QSetBorder::mousePressEvent(QMouseEvent * e)
{
	if (e->y() > e->x()) {
		if (e->y() < height() - e->x()) {
			if (left_.enabled) {
				setLeft(!left_.set);
				emit leftSet(left_.set);
			}
		} else {
			if (bottom_.enabled) {
				setBottom(!bottom_.set);
				emit bottomSet(bottom_.set);
			}
		}
	} else {
		if (e->y() < height() - e->x()) {
			if (top_.enabled) {
				setTop(!top_.set);
				emit topSet(top_.set);
			}
		} else {
			if (right_.enabled) {
				setRight(!right_.set);
				emit rightSet(right_.set);
			}
		}
	}
	update();
	emit clicked();
}


void QSetBorder::drawLine(QColor const & col, int x, int y, int x2, int y2)
{
	QPainter paint;
	paint.begin(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(col);
	paint.setPen(p);
	paint.drawLine(x, y, x2, y2);
	paint.end();
}


void QSetBorder::drawLeft(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!left_.enabled)
		col = QColor("grey");
	drawLine(col, m + l, m + l + 2, m + l, h - m - l - 1);
}
 

void QSetBorder::drawRight(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!right_.enabled)
		col = QColor("grey");
	drawLine(col, h - m - l + 1, m + l + 2, h - m - l + 1, h - m - l - 1);
}

 
void QSetBorder::drawTop(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!top_.enabled)
		col = QColor("grey");
	drawLine(col, m + l + 2, m + l, w - m - l - 1, m + l);
}


void QSetBorder::drawBottom(bool draw)
{
	QColor col(draw ? Qt::black : Qt::white);
	if (!bottom_.enabled)
		col = QColor("grey");
	drawLine(col, m + l + 2, w - m - l + 1, w - m - l - 1, w - m - l + 1);
}


void QSetBorder::setLeftEnabled(bool border)
{
	left_.enabled = border;
	drawLeft(border);
}


void QSetBorder::setRightEnabled(bool border)
{
	right_.enabled = border;
	drawRight(border);
}


void QSetBorder::setTopEnabled(bool border)
{
	top_.enabled = border;
	drawTop(border);
}


void QSetBorder::setBottomEnabled(bool border)
{
	bottom_.enabled = border;
	drawBottom(border);
}


void QSetBorder::setLeft(bool border)
{
	left_.set = border;
	drawLeft(border);
}


void QSetBorder::setRight(bool border)
{
	right_.set = border;
	drawRight(border);
}
 

void QSetBorder::setTop(bool border)
{
	top_.set = border;
	drawTop(border);
}
 

void QSetBorder::setBottom(bool border)
{
	bottom_.set = border;
	drawBottom(border);
}


void QSetBorder::setAll(bool border)
{
	setLeft(border);
	setRight(border);
	setTop(border);
	setBottom(border);
}


bool QSetBorder::getLeft()
{
	return left_.set;
}


bool QSetBorder::getRight()
{
	return right_.set;
}


bool QSetBorder::getTop()
{
	return top_.set;
}


bool QSetBorder::getBottom()
{
	return bottom_.set;
}
