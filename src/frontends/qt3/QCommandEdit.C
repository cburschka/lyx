/**
 * \file QCommandEdit.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QCommandEdit.h"

namespace lyx {
namespace frontend {

QCommandEdit::QCommandEdit(QWidget * parent)
	: QLineEdit(parent)
{
	setFocusPolicy(QWidget::ClickFocus);
}


void QCommandEdit::keyPressEvent(QKeyEvent * e)
{
	switch (e->key()) {
	case Key_Escape:
		escapePressed();
		break;

	case Key_Up:
		upPressed();
		break;

	case Key_Down:
		downPressed();
		break;

	default:
		QLineEdit::keyPressEvent(e);
		break;
	}
}


bool QCommandEdit::event(QEvent * e)
{
	if (e->type() != QEvent::KeyPress)
		return QLineEdit::event(e);

	QKeyEvent * ev = (QKeyEvent *)e;

	if (ev->key() != Key_Tab)
		return QLineEdit::event(e);

	tabPressed();
	return true;
}

} // namespace frontend
} // namespace lyx
