/**
 * \file GuiSearch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSearch.h"

#include "ControlSearch.h"
#include "qt_helpers.h"

#include <QLineEdit>
#include <QCloseEvent>

using std::string;


namespace lyx {
namespace frontend {

static void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = 0; i < box->count(); ++i) {
		if (box->itemText(i) == text)
			return;
	}

	box->addItem(text);
}


GuiSearchDialog::GuiSearchDialog(LyXView & lv)
	: GuiDialog(lv, "findreplace") 
{
	setupUi(this);
	setController(new ControlSearch(*this));
	setViewTitle(_("Find and Replace"));

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(findPB, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(replaceallPB, SIGNAL(clicked()), this, SLOT(replaceallClicked()));
	connect(findCO, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(findChanged()));

	setFocusProxy(findCO);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
	bc().addReadOnly(replaceCO);
	bc().addReadOnly(replacePB);
	bc().addReadOnly(replaceallPB);

	replacePB->setEnabled(false);
	replaceallPB->setEnabled(false);
}


ControlSearch & GuiSearchDialog::controller()
{
	return static_cast<ControlSearch &>(GuiDialog::controller());
}


void GuiSearchDialog::showView()
{
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
	GuiDialog::showView();
}


void GuiSearchDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiSearchDialog::findChanged()
{
	if (findCO->currentText().isEmpty()) {
		findPB->setEnabled(false);
		replacePB->setEnabled(false);
		replaceallPB->setEnabled(false);
	} else {
		findPB->setEnabled(true);
		replacePB->setEnabled(!controller().isBufferReadonly());
		replaceallPB->setEnabled(!controller().isBufferReadonly());
	}
}


void GuiSearchDialog::findClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	find(needle, caseCB->isChecked(), wordsCB->isChecked(),
		backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
}


void GuiSearchDialog::replaceClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	docstring const repl = qstring_to_ucs4(replaceCO->currentText());
	replace(needle, repl, caseCB->isChecked(), wordsCB->isChecked(),
		backwardsCB->isChecked(), false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchDialog::replaceallClicked()
{
	replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(), wordsCB->isChecked(), false, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchDialog::find(docstring const & str, bool casesens,
	bool words, bool backwards)
{
	controller().find(str, casesens, words, !backwards);
}


void GuiSearchDialog::replace(docstring const & findstr,
	docstring const & replacestr,
	bool casesens, bool words, bool backwards, bool all)
{
	controller().replace(findstr, replacestr, casesens, words,
			     !backwards, all);
}

} // namespace frontend
} // namespace lyx


#include "GuiSearch_moc.cpp"
