/**
 * \file QTabularCreate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "ControlTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QTabularCreate.h"
#include "Qt2BC.h"

#include <qspinbox.h>
#include <qpushbutton.h>

typedef QController<ControlTabularCreate, QView<QTabularCreateDialog> > base_class;


QTabularCreate::QTabularCreate(Dialog & parent)
	: base_class(parent, _("LyX: Insert Table"))
{
}


void QTabularCreate::build_dialog()
{
	dialog_.reset(new QTabularCreateDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QTabularCreate::apply()
{
	controller().params().first = dialog_->rowsSB->value();
	controller().params().second = dialog_->columnsSB->value();
}
