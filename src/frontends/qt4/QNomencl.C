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
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlCommand, QView<QNomenclDialog> > nomencl_base_class;


QNomencl::QNomencl(Dialog & parent, docstring const & title)
	: nomencl_base_class(parent, title)
{
}


void QNomencl::build_dialog()
{
	dialog_.reset(new QNomenclDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->symbolED);
	bcview().addReadOnly(dialog_->descriptionTE);
	bcview().addReadOnly(dialog_->prefixED);
}


void QNomencl::update_contents()
{
	dialog_->prefixED->setText(toqstr(controller().params()["prefix"]));
	dialog_->symbolED->setText(toqstr(controller().params()["symbol"]));
	QString description = toqstr(controller().params()["description"]);
	description.replace("\\\\","\n");
	dialog_->descriptionTE->setPlainText(description);

	bc().valid(isValid());
}


void QNomencl::apply()
{
	controller().params()["prefix"] = qstring_to_ucs4(dialog_->prefixED->text());
	controller().params()["symbol"] = qstring_to_ucs4(dialog_->symbolED->text());
	QString description = dialog_->descriptionTE->toPlainText();
	description.replace('\n',"\\\\");
	controller().params()["description"] = qstring_to_ucs4(description);
}


bool QNomencl::isValid()
{
	QString const description = dialog_->descriptionTE->toPlainText();
	return !dialog_->symbolED->text().isEmpty() && !description.isEmpty();
}

} // namespace frontend
} // namespace lyx
