/**
 * \file QSetBorder.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include "qsetborder.h"


QSetBorder::QSetBorder(QWidget * parent, char const * name, WFlags fl)
	: QWidget(parent, name, fl),
	  left_(true), right_(true), top_(true), bottom_(true),
	  buffer(75, 75)
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
			setLeft(!left_);
			emit leftSet(left_);
		} else {
			setBottom(!bottom_);
			emit bottomSet(bottom_);
		}
	} else {
		if (e->y() < height() - e->x()) {
			setTop(!top_);
			emit topSet(top_);
		} else {
			setRight(!right_);
			emit rightSet(right_);
		}
	}
	update();
	emit clicked();
}


void QSetBorder::drawLine(bool draw, int x, int y, int x2, int y2)
{
	QPainter paint;
	paint.begin(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(draw ? Qt::black : Qt::white);
	paint.setPen(p);
	paint.drawLine(x, y, x2, y2);
	paint.end();
}


void QSetBorder::drawLeft(bool draw)
{
	drawLine(draw, m + l, m + l + 2, m + l, h - m - l - 1);
}
 

void QSetBorder::drawRight(bool draw)
{
	drawLine(draw, h - m - l + 1, m + l + 2, h - m - l + 1, h - m - l - 1);
}

 
void QSetBorder::drawTop(bool draw)
{
	drawLine(draw, m + l + 2, m + l, w - m - l - 1, m + l);
}


void QSetBorder::drawBottom(bool draw)
{
	drawLine(draw, m + l + 2, w - m - l + 1, w - m - l - 1, w - m - l + 1);
}

 
void QSetBorder::setLeft(bool border)
{
	left_ = border;
	drawLeft(border);
}


void QSetBorder::setRight(bool border)
{
	right_ = border;
	drawRight(border);
}
 

void QSetBorder::setTop(bool border)
{
	top_ = border;
	drawTop(border);
}
 

void QSetBorder::setBottom(bool border)
{
	bottom_ = border;
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
	return left_;
}


bool QSetBorder::getRight()
{
	return right_;
}


bool QSetBorder::getTop()
{
	return top_;
}


bool QSetBorder::getBottom()
{
	return bottom_;
}
