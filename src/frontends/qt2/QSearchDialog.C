/**
 * \file QSearchDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QSearchDialog.h"
#include "QSearch.h"
#include "qt_helpers.h"

#include "controllers/ControlSearch.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

using std::string;

namespace lyx {
namespace frontend {

namespace {

void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = 0; i < box->count(); ++i) {
		if (box->text(i) == text)
			return;
	}

	box->insertItem(text);
}

};


QSearchDialog::QSearchDialog(QSearch * form)
	: QSearchDialogBase(qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget(), 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


void QSearchDialog::show()
{
	QSearchDialogBase::show();
	findCO->setFocus();
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
}


void QSearchDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QSearchDialog::findChanged()
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


void QSearchDialog::findClicked()
{
	string const find(fromqstr(findCO->currentText()));
	form_->find(find,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
}


void QSearchDialog::replaceClicked()
{
	string const find(fromqstr(findCO->currentText()));
	string const replace(fromqstr(replaceCO->currentText()));
	form_->replace(find, replace,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked(), false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void QSearchDialog::replaceallClicked()
{
	form_->replace(fromqstr(findCO->currentText()),
		fromqstr(replaceCO->currentText()),
		caseCB->isChecked(),
		wordsCB->isChecked(),
		false, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}

} // namespace frontend
} // namespace lyx
