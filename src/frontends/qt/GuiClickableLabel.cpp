// -*- C++ -*-
/**
 * \file GuiClickableLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#include "config.h"

#include "GuiClickableLabel.h"

#include <QMouseEvent>

namespace lyx {
namespace frontend {

GuiClickableLabel::GuiClickableLabel(QWidget * parent)
	: QLabel(parent)
{}

GuiClickableLabel::~GuiClickableLabel()
{}

void GuiClickableLabel::mousePressEvent(QMouseEvent * e) {
	if (e->button() == Qt::LeftButton)
		Q_EMIT pressed();
}

void GuiClickableLabel::mouseReleaseEvent(QMouseEvent *) {
	Q_EMIT clicked();
}

}
}

#include "moc_GuiClickableLabel.cpp"
