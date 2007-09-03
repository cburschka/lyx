/**
 * \file GuiBibitem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBibitem.h"
#include "qt_helpers.h"

#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiBibItemDialog
//
/////////////////////////////////////////////////////////////////////

GuiBibitemDialog::GuiBibitemDialog(GuiBibitem * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(keyED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(labelED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
}


void GuiBibitemDialog::change_adaptor()
{
	form_->changed();
}


void GuiBibitemDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// GuiBibItem
//
/////////////////////////////////////////////////////////////////////


GuiBibitem::GuiBibitem(GuiDialog & parent)
	: GuiView<GuiBibitemDialog>(parent, _("Bibliography Entry Settings"))
{
}


void GuiBibitem::build_dialog()
{
	dialog_.reset(new GuiBibitemDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->keyED);
	bc().addReadOnly(dialog_->labelED);
}


void GuiBibitem::update_contents()
{
	dialog_->keyED->setText(toqstr(controller().params()["key"]));
	dialog_->labelED->setText(toqstr(controller().params()["label"]));
}


void GuiBibitem::applyView()
{
	controller().params()["key"] = qstring_to_ucs4(dialog_->keyED->text());
	controller().params()["label"] = qstring_to_ucs4(dialog_->labelED->text());
}


bool GuiBibitem::isValid()
{
	return !dialog_->keyED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiBibitem_moc.cpp"
