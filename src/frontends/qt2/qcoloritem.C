/**
 * \file qcoloritem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include "qcoloritem.h"

#include <qfont.h> 
#include <qfontmetrics.h>
#include <qpainter.h>
 
QColorItem::QColorItem(QColor c, QString const & t)
	: color_(c), text_(t)
{
	QFont font;
	QFontMetrics metrics(font);
 
	ascent_ = metrics.ascent();
	width_ = 40 + metrics.width(t);
	height_ = metrics.ascent() + metrics.descent() + 6;
}

 
void QColorItem::paint(QPainter * p)
{
	p->fillRect(2, 2, 35, height_, color_);
	p->drawText(40, 3 + ascent_, text_);
}
