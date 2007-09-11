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

GuiNomenclDialog::GuiNomenclDialog(LyXView & lv)
	: GuiDialog(lv, "nomenclature")
{
	setupUi(this);
	setController(new ControlCommand(*this, "nomenclature", "nomenclature"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(symbolED, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(descriptionTE, SIGNAL(textChanged()),
		this, SLOT(change_adaptor()));

	setFocusProxy(descriptionTE);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(symbolED);
	bc().addReadOnly(descriptionTE);
	bc().addReadOnly(prefixED);
}


ControlCommand & GuiNomenclDialog::controller() const
{
	return static_cast<ControlCommand &>(GuiDialog::controller());
}


void GuiNomenclDialog::change_adaptor()
{
	changed();
}


void GuiNomenclDialog::reject()
{
	slotClose();
}


void GuiNomenclDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiNomenclDialog::update_contents()
{
	prefixED->setText(toqstr(controller().params()["prefix"]));
	symbolED->setText(toqstr(controller().params()["symbol"]));
	QString description = toqstr(controller().params()["description"]);
	description.replace("\\\\","\n");
	descriptionTE->setPlainText(description);

	bc().setValid(isValid());
}


void GuiNomenclDialog::applyView()
{
	controller().params()["prefix"] = qstring_to_ucs4(prefixED->text());
	controller().params()["symbol"] = qstring_to_ucs4(symbolED->text());
	QString description = descriptionTE->toPlainText();
	description.replace('\n',"\\\\");
	controller().params()["description"] = qstring_to_ucs4(description);
}


bool GuiNomenclDialog::isValid()
{
	QString const description = descriptionTE->toPlainText();
	return !symbolED->text().isEmpty() && !description.isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiNomencl_moc.cpp"
