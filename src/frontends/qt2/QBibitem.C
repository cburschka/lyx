/**
 * \file QBibitem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "support/lstrings.h"

#include "ControlBibitem.h"
#include "qt_helpers.h"
#include "debug.h"

#include <qlineedit.h>
#include <qpushbutton.h>

#include "QBibitemDialog.h"
#include "QBibitem.h"
#include "Qt2BC.h"

typedef Qt2CB<ControlBibitem, Qt2DB<QBibitemDialog> > base_class;


QBibitem::QBibitem()
	: base_class(qt_("Bibliography Item"))
{
}


void QBibitem::build_dialog()
{
	dialog_.reset(new QBibitemDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->keyED);
	bc().addReadOnly(dialog_->labelED);
}


void QBibitem::update_contents()
{
	dialog_->keyED->setText(toqstr(controller().params().getContents()));
	dialog_->labelED->setText(toqstr(controller().params().getOptions()));
}


void QBibitem::apply()
{
	controller().params().setContents(fromqstr(dialog_->keyED->text()));
	controller().params().setOptions(fromqstr(dialog_->labelED->text()));
}


bool QBibitem::isValid()
{
	return !dialog_->keyED->text().isEmpty();
}
