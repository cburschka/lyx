/**
 * \file QNomencl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ControlCommand.h"
#include "qt_helpers.h"

#include "QNomenclDialog.h"
#include "QNomencl.h"
#include "Qt2BC.h"
#include "ButtonController.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlCommand, QView<QNomenclDialog> > base_class;


QNomencl::QNomencl(Dialog & parent, docstring const & title)
	: base_class(parent, title)
{
}


void QNomencl::build_dialog()
{
	dialog_.reset(new QNomenclDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->symbolED);
	bcview().addReadOnly(dialog_->descrED);
	bcview().addReadOnly(dialog_->prefixED);
}


void QNomencl::update_contents()
{
	dialog_->prefixED->setText(toqstr(controller().params()["prefix"]));
	dialog_->symbolED->setText(toqstr(controller().params()["symbol"]));
	dialog_->descrED->setText(toqstr(controller().params()["description"]));

	bc().valid(isValid());
}


void QNomencl::apply()
{
	controller().params()["prefix"] = qstring_to_ucs4(dialog_->prefixED->text());
	controller().params()["symbol"] = qstring_to_ucs4(dialog_->symbolED->text());
	controller().params()["description"] = qstring_to_ucs4(dialog_->descrED->text());
}


bool QNomencl::isValid()
{
	return (!dialog_->symbolED->text().isEmpty() && !dialog_->descrED->text().isEmpty());
}

} // namespace frontend
} // namespace lyx
