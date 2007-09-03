/**
 * \file GuiNomencl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiNomencl.h"

#include "debug.h"
#include "ControlCommand.h"
#include "qt_helpers.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWhatsThis>
#include <QCloseEvent>

using std::string;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiNomenclDialog
//
/////////////////////////////////////////////////////////////////////

GuiNomenclDialog::GuiNomenclDialog(GuiNomencl * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(symbolED, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(descriptionTE, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));

	setFocusProxy(descriptionTE);
}


void GuiNomenclDialog::show()
{
	QDialog::show();
}


void GuiNomenclDialog::change_adaptor()
{
	form_->changed();
}


void GuiNomenclDialog::reject()
{
	form_->slotClose();
}


void GuiNomenclDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// GuiNomencl
//
/////////////////////////////////////////////////////////////////////


GuiNomencl::GuiNomencl(GuiDialog & parent, docstring const & title)
	: GuiView<GuiNomenclDialog>(parent, title)
{
}


void GuiNomencl::build_dialog()
{
	dialog_.reset(new GuiNomenclDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->symbolED);
	bc().addReadOnly(dialog_->descriptionTE);
	bc().addReadOnly(dialog_->prefixED);
}


void GuiNomencl::update_contents()
{
	dialog_->prefixED->setText(toqstr(controller().params()["prefix"]));
	dialog_->symbolED->setText(toqstr(controller().params()["symbol"]));
	QString description = toqstr(controller().params()["description"]);
	description.replace("\\\\","\n");
	dialog_->descriptionTE->setPlainText(description);

	bc().setValid(isValid());
}


void GuiNomencl::apply()
{
	controller().params()["prefix"] = qstring_to_ucs4(dialog_->prefixED->text());
	controller().params()["symbol"] = qstring_to_ucs4(dialog_->symbolED->text());
	QString description = dialog_->descriptionTE->toPlainText();
	description.replace('\n',"\\\\");
	controller().params()["description"] = qstring_to_ucs4(description);
}


bool GuiNomencl::isValid()
{
	QString const description = dialog_->descriptionTE->toPlainText();
	return !dialog_->symbolED->text().isEmpty() && !description.isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiNomencl_moc.cpp"
