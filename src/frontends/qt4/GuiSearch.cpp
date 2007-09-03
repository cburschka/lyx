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
#include "qt_helpers.h"

#include <QLineEdit>
#include <QCloseEvent>

using std::string;

namespace lyx {
namespace frontend {


/////////////////////////////////////////////////////////////////////
//
// GuiSearchDialog
//
/////////////////////////////////////////////////////////////////////


static void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = 0; i < box->count(); ++i) {
		if (box->itemText(i) == text)
			return;
	}

	box->addItem(text);
}


GuiSearchDialog::GuiSearchDialog(GuiSearch * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));
	connect(findPB, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(replaceallPB, SIGNAL(clicked()), this, SLOT(replaceallClicked()));
	connect(findCO, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(findChanged()));

	setFocusProxy(findCO);
}


void GuiSearchDialog::showView()
{
	QDialog::show();
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
}


void GuiSearchDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
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
		replacePB->setEnabled(!form_->readOnly());
		replaceallPB->setEnabled(!form_->readOnly());
	}
}


void GuiSearchDialog::findClicked()
{
	docstring const find = qstring_to_ucs4(findCO->currentText());
	form_->find(find,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
}


void GuiSearchDialog::replaceClicked()
{
	docstring const find = qstring_to_ucs4(findCO->currentText());
	docstring const replace = qstring_to_ucs4(replaceCO->currentText());
	form_->replace(find, replace,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked(), false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchDialog::replaceallClicked()
{
	form_->replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(),
		wordsCB->isChecked(),
		false, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


/////////////////////////////////////////////////////////////////////
//
// GuiSearch
//
/////////////////////////////////////////////////////////////////////


GuiSearch::GuiSearch(GuiDialog & parent)
	: GuiView<GuiSearchDialog>(parent, _("Find and Replace"))
{
}


void GuiSearch::build_dialog()
{
	dialog_.reset(new GuiSearchDialog(this));

	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->replaceCO);
	bc().addReadOnly(dialog_->replacePB);
	bc().addReadOnly(dialog_->replaceallPB);

	dialog_->replacePB->setEnabled(false);
	dialog_->replaceallPB->setEnabled(false);
}


void GuiSearch::find(docstring const & str, bool casesens,
		   bool words, bool backwards)
{
	controller().find(str, casesens, words, !backwards);
}


void GuiSearch::replace(docstring const & findstr, docstring const & replacestr,
	bool casesens, bool words, bool backwards, bool all)
{
	controller().replace(findstr, replacestr, casesens, words,
			     !backwards, all);
}

} // namespace frontend
} // namespace lyx


#include "GuiSearch_moc.cpp"
