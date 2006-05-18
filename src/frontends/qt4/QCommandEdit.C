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
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>

namespace lyx {
namespace frontend {

QCommandEdit::QCommandEdit(QWidget * parent)
	: QLineEdit(parent)
{
	setFocusPolicy(Qt::ClickFocus);
}


void QCommandEdit::keyPressEvent(QKeyEvent * e)
{
	switch (e->key()) {
	case Qt::Key_Escape:
		emit escapePressed();
		break;

	case Qt::Key_Up:
		emit upPressed();
		break;

	case Qt::Key_Down:
		emit downPressed();
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

	if (ev->key() != Qt::Key_Tab)
		return QLineEdit::event(e);

	emit tabPressed();
	return true;
}

} // namespace frontend
} // namespace lyx

#include "QCommandEdit_moc.cpp"
