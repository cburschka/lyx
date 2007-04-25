/**
 * \file Qt2BC.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Qt2BC.h"
#include "qt_helpers.h"

#include <QPushButton>
#include <QLineEdit>

namespace lyx {
namespace frontend {

Qt2BC::Qt2BC(ButtonController const & parent,
	     docstring const & cancel, docstring const & close)
	: GuiBC<QPushButton, QWidget>(parent, cancel, close)
{}


void Qt2BC::setButtonEnabled(QPushButton * obj, bool enabled) const
{
	obj->setEnabled(enabled);
}


void Qt2BC::setWidgetEnabled(QWidget * obj, bool enabled) const
{
	// yuck, rtti, but the user comes first
	if (QLineEdit * le = qobject_cast<QLineEdit*>(obj))
		le->setReadOnly(!enabled);
	else
		obj->setEnabled(enabled);

	obj->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}


void Qt2BC::setButtonLabel(QPushButton * obj, docstring const & label) const
{
	obj->setText(toqstr(label));
}

} // namespace frontend
} // namespace lyx
