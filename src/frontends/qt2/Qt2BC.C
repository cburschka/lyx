/**
 * \file Qt2BC.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae 
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Qt2BC.h"
#include "debug.h"

#include <qbutton.h>
#include <qlineedit.h>

Qt2BC::Qt2BC(string const & cancel, string const & close)
	: GuiBC<QButton, QWidget>(cancel, close)
{}


void Qt2BC::setButtonEnabled(QButton * obj, bool enabled)
{
	obj->setEnabled(enabled);
}


void Qt2BC::setWidgetEnabled(QWidget * obj, bool enabled)
{
	// yuck, rtti, but the user comes first
	if (obj->inherits("QLineEdit")) {
		QLineEdit * le(static_cast<QLineEdit*>(obj));
		le->setReadOnly(!enabled);
	} else {
		obj->setEnabled(enabled);
	}

	QWidget::FocusPolicy const p =
		(enabled) ? QWidget::StrongFocus : QWidget::NoFocus;
	obj->setFocusPolicy(p);
}


void Qt2BC::setButtonLabel(QButton * obj, string const & label)
{
	obj->setText(label.c_str());
}
