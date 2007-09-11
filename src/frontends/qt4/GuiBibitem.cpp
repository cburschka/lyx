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
#include "ControlCommand.h"
#include "qt_helpers.h"

#include <QCloseEvent>
#include <QLineEdit>
#include <QPushButton>


namespace lyx {
namespace frontend {


GuiBibitemDialog::GuiBibitemDialog(LyXView & lv)
	: GuiDialog(lv, "bibitem")
{
	setupUi(this);
	setViewTitle(_("Bibliography Entry Settings"));
	setController(new ControlCommand(*this, "bibitem", "bibitem"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(keyED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));
	connect(labelED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::OkCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(keyED);
	bc().addReadOnly(labelED);
}


ControlCommand & GuiBibitemDialog::controller() const
{
	return static_cast<ControlCommand &>(GuiDialog::controller());
}


void GuiBibitemDialog::change_adaptor()
{
	changed();
}


void GuiBibitemDialog::closeEvent(QCloseEvent *e)
{
	slotClose();
	e->accept();
}


void GuiBibitemDialog::update_contents()
{
	keyED->setText(toqstr(controller().params()["key"]));
	labelED->setText(toqstr(controller().params()["label"]));
}


void GuiBibitemDialog::applyView()
{
	controller().params()["key"] = qstring_to_ucs4(keyED->text());
	controller().params()["label"] = qstring_to_ucs4(labelED->text());
}


bool GuiBibitemDialog::isValid()
{
	return !keyED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiBibitem_moc.cpp"
