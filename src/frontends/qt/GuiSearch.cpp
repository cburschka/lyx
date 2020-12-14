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

#include "lyxfind.h"
#include "qt_helpers.h"
#include "FuncRequest.h"
#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "GuiView.h"

#include "support/gettext.h"
#include "frontends/alert.h"

#include <QLineEdit>
#include <QShowEvent>

using namespace std;

namespace lyx {
namespace frontend {

static void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = box->count(); --i >= 0; )
		if (box->itemText(i) == text)
			return;

	box->insertItem(0, text);
}


GuiSearch::GuiSearch(GuiView & lv)
	: GuiDialog(lv, "findreplace", qt_("Find and Replace"))
{
	setupUi(this);

	// fix height to minimum
	setFixedHeight(sizeHint().height());

	// align items in grid on top
	mainGridLayout->setAlignment(Qt::AlignTop);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));
	connect(findPB, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(replaceallPB, SIGNAL(clicked()), this, SLOT(replaceallClicked()));
	connect(findCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(findChanged()));

	setFocusProxy(findCO);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(buttonBox->button(QDialogButtonBox::Close));

	findCO->setCompleter(0);
	replaceCO->setCompleter(0);

	replacePB->setEnabled(false);
	replaceallPB->setEnabled(false);
}


void GuiSearch::showEvent(QShowEvent * e)
{
	findChanged();
	findPB->setFocus();
	findCO->lineEdit()->selectAll();
	GuiDialog::showEvent(e);
}


void GuiSearch::findChanged()
{
	findPB->setEnabled(!findCO->currentText().isEmpty());
	bool const replace = !findCO->currentText().isEmpty() && !isBufferReadonly();
	replacePB->setEnabled(replace);
	replaceallPB->setEnabled(replace);
	replaceLA->setEnabled(replace);
	replaceCO->setEnabled(replace);
}


void GuiSearch::findClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	find(needle, caseCB->isChecked(), wordsCB->isChecked(),
		!backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	findCO->lineEdit()->selectAll();
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
	docstring const sdata =
		find2string(search, casesensitive, matchword, forward);
	dispatch(FuncRequest(LFUN_WORD_FIND, sdata));
}


void GuiSearch::replace(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	docstring const sdata =
		replace2string(replace, search, casesensitive,
				     matchword, all, forward);
	dispatch(FuncRequest(LFUN_WORD_REPLACE, sdata));
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiSearch.cpp"
