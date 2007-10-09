/**
 * \file GuiSearch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSearch.h"

#include "FuncRequest.h"
#include "lyxfind.h"

#include "qt_helpers.h"

#include <QLineEdit>
#include <QCloseEvent>

using std::string;


namespace lyx {
namespace frontend {

static void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = box->count(); --i >= 0; )
		if (box->itemText(i) == text)
			return;

	box->addItem(text);
}


GuiSearch::GuiSearch(LyXView & lv)
	: GuiDialog(lv, "findreplace")
{
	setupUi(this);
	setViewTitle(_("Find and Replace"));

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(findPB, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(replaceallPB, SIGNAL(clicked()), this, SLOT(replaceallClicked()));
	connect(findCO, SIGNAL(editTextChanged(QString)),
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


void GuiSearch::showView()
{
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
	GuiDialog::showView();
}


void GuiSearch::closeEvent(QCloseEvent * e)
{
	slotClose();
	GuiDialog::closeEvent(e);
}


void GuiSearch::findChanged()
{
	if (findCO->currentText().isEmpty()) {
		findPB->setEnabled(false);
		replacePB->setEnabled(false);
		replaceallPB->setEnabled(false);
	} else {
		findPB->setEnabled(true);
		replacePB->setEnabled(!isBufferReadonly());
		replaceallPB->setEnabled(!isBufferReadonly());
	}
}


void GuiSearch::findClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	find(needle, caseCB->isChecked(), wordsCB->isChecked(),
		!backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
}


void GuiSearch::replaceClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	docstring const repl = qstring_to_ucs4(replaceCO->currentText());
	replace(needle, repl, caseCB->isChecked(), wordsCB->isChecked(),
		!backwardsCB->isChecked(), false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearch::replaceallClicked()
{
	replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(), wordsCB->isChecked(), true, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearch::find(docstring const & search, bool casesensitive,
			 bool matchword, bool forward)
{
	docstring const data =
		find2string(search, casesensitive, matchword, forward);
	dispatch(FuncRequest(LFUN_WORD_FIND, data));
}


void GuiSearch::replace(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	docstring const data =
		replace2string(search, replace, casesensitive,
				     matchword, all, forward);
	dispatch(FuncRequest(LFUN_WORD_REPLACE, data));
}

Dialog * createGuiSearch(LyXView & lv) { return new GuiSearch(lv); }


} // namespace frontend
} // namespace lyx


#include "GuiSearch_moc.cpp"
