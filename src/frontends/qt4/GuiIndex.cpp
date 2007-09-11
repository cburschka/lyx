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

/////////////////////////////////////////////////////////////////
//
// Base implementation
//
/////////////////////////////////////////////////////////////////

GuiIndexDialogBase::GuiIndexDialogBase(LyXView & lv,
		docstring const & title, QString const & label, std::string const & name)
	: GuiDialog(lv, name)
{
	label_ = label;
	setupUi(this);
	setViewTitle(title);
	setController(new ControlCommand(*this, name, name));

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


ControlCommand & GuiIndexDialogBase::controller()
{
	return static_cast<ControlCommand &>(GuiDialog::controller());
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
	slotClose();
	e->accept();
}


void GuiIndexDialogBase::updateContents()
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


/////////////////////////////////////////////////////////////////
//
// Index Dialog
//
/////////////////////////////////////////////////////////////////


GuiIndexDialog::GuiIndexDialog(LyXView & lv)
	: GuiIndexDialogBase(lv, _("Index Entry"), qt_("&Keyword:"), "index") 
{
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
}


/////////////////////////////////////////////////////////////////
//
// Label Dialog
//
/////////////////////////////////////////////////////////////////

GuiLabelDialog::GuiLabelDialog(LyXView & lv)
	: GuiIndexDialogBase(lv, _("Label"), qt_("&Label:"), "label")
{}


} // namespace frontend
} // namespace lyx

#include "GuiIndex_moc.cpp"
