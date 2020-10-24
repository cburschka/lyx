/**
 * \file GuiFontExample.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiFontExample.h"
#include "GuiFontMetrics.h"

#include <QPainter>
#include <QPaintEvent>


//namespace lyx {

void GuiFontExample::set(QFont const & font, QString const & text)
{
	font_ = font;
	text_ = text;
	update();
}


QSize GuiFontExample::sizeHint() const
{
	lyx::frontend::GuiFontMetrics m(font_);
	return QSize(m.width(text_) + 10, m.maxHeight() + 6);
}


void GuiFontExample::paintEvent(QPaintEvent *)
{
	QPainter p;
	lyx::frontend::GuiFontMetrics m(font_);

	p.begin(this);
	p.setFont(font_);
	p.drawRect(0, 0, width() - 1, height() - 1);
	p.drawText(5, 3 + m.maxAscent(), text_);
	p.end();
}


//} // namespace lyx
