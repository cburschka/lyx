/**
 * \file QCommandEdit.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "QCommandEdit.h"
 
QCommandEdit::QCommandEdit(QWidget * parent)
	: QLineEdit(parent)
{ 
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
			if (e->state() & ControlButton)
				emit rightPressed();
			else
				QLineEdit::keyPressEvent(e);
			break;

		default:
			QLineEdit::keyPressEvent(e);
			break;
	}
}
