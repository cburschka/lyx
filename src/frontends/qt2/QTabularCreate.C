/**
 * \file QTabularCreate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QTabularCreate.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "debug.h"

#include <qspinbox.h>
#include <qpushbutton.h>

typedef QController<ControlTabularCreate, QView<QTabularCreateDialog> > base_class;


QTabularCreate::QTabularCreate(Dialog & parent)
	: base_class(parent, qt_("LyX: Insert Table"))
{
}


void QTabularCreate::build_dialog()
{
	dialog_.reset(new QTabularCreateDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
}


void QTabularCreate::apply()
{
	controller().params().first = dialog_->rowsSB->value();
	controller().params().second = dialog_->columnsSB->value();
}
