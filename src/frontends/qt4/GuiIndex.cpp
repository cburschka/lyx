/**
 * \file GuiIndex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiIndex.h"

#include "ControlCommand.h"

#include "debug.h"
#include "qt_helpers.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCloseEvent>

using std::string;


namespace lyx {
namespace frontend {

GuiIndexDialogBase::GuiIndexDialogBase(LyXView & lv,
		docstring const & title, QString const & label)
	: GuiDialog(lv, "index")
{
	label_ = label;
	setupUi(this);
	setViewTitle(title);
	setController(new ControlCommand(*this, "index", "index"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(keywordED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor()));

	setFocusProxy(keywordED);

	keywordLA->setText(label_);

	keywordED->setWhatsThis( qt_(
		"The format of the entry in the index.\n"
		"\n"
		"An entry can be specified as a sub-entry of\n"
		"another with \"!\":\n"
		"\n"
		"cars!mileage\n"
		"\n"
		"You can cross-refer to another entry like so:\n"
		"\n"
		"cars!mileage|see{economy}\n"
		"\n"
		"For further details refer to the local LaTeX\n"
		"documentation.\n")
	);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(keywordED);
}


ControlCommand & GuiIndexDialogBase::controller() const
{
	return static_cast<ControlCommand &>(Dialog::controller());
}


void GuiIndexDialogBase::change_adaptor()
{
	changed();
}


void GuiIndexDialogBase::reject()
{
	slotClose();
}


void GuiIndexDialogBase::closeEvent(QCloseEvent * e)
{
	slotWMHide();
	e->accept();
}


void GuiIndexDialogBase::update_contents()
{
	docstring const contents = controller().params()["name"];
	keywordED->setText(toqstr(contents));
	bc().setValid(!contents.empty());
}


void GuiIndexDialogBase::applyView()
{
	controller().params()["name"] = qstring_to_ucs4(keywordED->text());
}


bool GuiIndexDialogBase::isValid()
{
	return !keywordED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "GuiIndex_moc.cpp"
