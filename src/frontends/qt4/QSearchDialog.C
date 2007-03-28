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
//Added by qt3to4:
#include <QCloseEvent>

#include "controllers/ControlSearch.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>


namespace lyx {
namespace frontend {

namespace {

void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = 0; i < box->count(); ++i) {
		if (box->itemText(i) == text)
			return;
	}

	box->addItem(text);
}

};


QSearchDialog::QSearchDialog(QSearch * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));

    connect( findPB, SIGNAL( clicked() ), this, SLOT( findClicked() ) );
    connect( replacePB, SIGNAL( clicked() ), this, SLOT( replaceClicked() ) );
    connect( replaceallPB, SIGNAL( clicked() ), this, SLOT( replaceallClicked() ) );
    connect( findCO, SIGNAL( editTextChanged(const QString&) ), this, SLOT( findChanged() ) );

	setFocusProxy(findCO);
}


void QSearchDialog::show()
{
	QDialog::show();
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
	docstring const find(qstring_to_ucs4(findCO->currentText()));
	form_->find(find,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
}


void QSearchDialog::replaceClicked()
{
	docstring const find(qstring_to_ucs4(findCO->currentText()));
	docstring const replace(qstring_to_ucs4(replaceCO->currentText()));
	form_->replace(find, replace,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked(), false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void QSearchDialog::replaceallClicked()
{
	form_->replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(),
		wordsCB->isChecked(),
		false, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}

} // namespace frontend
} // namespace lyx

#include "QSearchDialog_moc.cpp"
