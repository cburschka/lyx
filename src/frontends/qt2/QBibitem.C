/**
 * \file QBibitem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlCommand.h"
#include "qt_helpers.h"

#include <qlineedit.h>
#include <qpushbutton.h>

#include "QBibitemDialog.h"
#include "QBibitem.h"
#include "Qt2BC.h"

namespace lyx {
namespace frontend {

typedef QController<ControlCommand, QView<QBibitemDialog> > base_class;


QBibitem::QBibitem(Dialog & parent)
	: base_class(parent, _("LyX: Bibliography Item Settings"))
{
}


void QBibitem::build_dialog()
{
	dialog_.reset(new QBibitemDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->keyED);
	bcview().addReadOnly(dialog_->labelED);
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

} // namespace frontend
} // namespace lyx
