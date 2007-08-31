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

#include "debug.h"
#include "ControlCommand.h"
#include "qt_helpers.h"

#include "GuiNomencl.h"
#include "Qt2BC.h"
#include "ButtonController.h"
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


typedef QController<ControlCommand, GuiView<GuiNomenclDialog> > NomenBase;


GuiNomencl::GuiNomencl(Dialog & parent, docstring const & title)
	: NomenBase(parent, title)
{
}


void GuiNomencl::build_dialog()
{
	dialog_.reset(new GuiNomenclDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->symbolED);
	bcview().addReadOnly(dialog_->descriptionTE);
	bcview().addReadOnly(dialog_->prefixED);
}


void GuiNomencl::update_contents()
{
	dialog_->prefixED->setText(toqstr(controller().params()["prefix"]));
	dialog_->symbolED->setText(toqstr(controller().params()["symbol"]));
	QString description = toqstr(controller().params()["description"]);
	description.replace("\\\\","\n");
	dialog_->descriptionTE->setPlainText(description);

	bc().valid(isValid());
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
