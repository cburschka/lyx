/**
 * \file QCommandEdit.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include "QCommandEdit.h"
 
#ifdef __GNUG__
#pragma implementation
#endif

QCommandEdit::QCommandEdit(QWidget * parent)
	: QLineEdit(parent)
{ 
	setFocusPolicy(QWidget::ClickFocus); 
}


void QCommandEdit::keyPressEvent(QKeyEvent * e)
{
	switch (e->key()) {
		case Key_Escape:
			emit escapePressed();
			break;
 
		case Key_Up:
			emit upPressed();
			break;

		case Key_Down:
			emit downPressed();
			break;

		case Key_Right:
			if (cursorPosition() == text().length())
				emit rightPressed();
			else
				QLineEdit::keyPressEvent(e);
			break;

		default:
			QLineEdit::keyPressEvent(e);
			break;
	}
}
