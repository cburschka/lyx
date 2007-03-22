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

#include "QBibitem.h"
#include "QBibitemDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlCommand.h"

#include <qlineedit.h>
#include <qpushbutton.h>


namespace lyx {
namespace frontend {

typedef QController<ControlCommand, QView<QBibitemDialog> > bibitem_base_class;


QBibitem::QBibitem(Dialog & parent)
	: bibitem_base_class(parent, _("Bibliography Entry Settings"))
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
	dialog_->keyED->setText(toqstr(controller().params()["key"]));
	dialog_->labelED->setText(toqstr(controller().params()["label"]));
}


void QBibitem::apply()
{
	controller().params()["key"] = qstring_to_ucs4(dialog_->keyED->text());
	controller().params()["label"] = qstring_to_ucs4(dialog_->labelED->text());
}


bool QBibitem::isValid()
{
	return !dialog_->keyED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx
