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
	  left_(false), right_(false), top_(false), bottom_(false), buffer(75,75)
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
			drawLeft(!left_);
			left_ = !left_;
			emit leftSet(left_);
		} else {
			drawBottom(!bottom_);
			bottom_ = !bottom_;
			emit bottomSet(bottom_);
		}
	} else {
		if (e->y() < height() - e->x()) {
			drawTop(!top_);
			top_ = !top_;
			emit topSet(top_);
		} else {
			drawRight(!right_);
			right_ = !right_;
			emit rightSet(right_);
		}
	}
	update();
}


void QSetBorder::drawLeft(bool draw)
{
	QPainter paint;
	paint.begin(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(draw ? Qt::black : Qt::white);
	paint.setPen(p);
	paint.drawLine(m + l, m + l + 2, m + l, h - m - l - 1);
	paint.end();
}
 

void QSetBorder::drawRight(bool draw)
{
	QPainter paint;
	paint.begin(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(draw ? Qt::black : Qt::white);
	paint.setPen(p);
	paint.drawLine(h - m - l + 1, m + l + 2, h - m - l + 1, h - m - l - 1);
	paint.end();
}

void QSetBorder::drawTop(bool draw)
{
	QPainter paint;
	paint.begin(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(draw ? Qt::black : Qt::white);
	paint.setPen(p);
	paint.drawLine(m + l + 2, m + l, w - m - l - 1, m + l);
	paint.end();
}

void QSetBorder::drawBottom(bool draw)
{
	QPainter paint;
	paint.begin(&buffer);
	QPen p = paint.pen();
	p.setWidth(2);
	p.setColor(draw ? Qt::black : Qt::white);
	paint.setPen(p);
	paint.drawLine(m + l + 2, w - m - l + 1, w - m - l - 1, w - m - l + 1);
	paint.end();
}

void QSetBorder::setLeft(bool border)
{
	left_ = border;
}

void QSetBorder::setRight(bool border)
{
	right_ = border;
}

void QSetBorder::setTop(bool border)
{
	top_ = border;
}

void QSetBorder::setBottom(bool border)
{
	bottom_ = border;
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
