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
#include <QComboBox>

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

	Q_CONNECT_1(QPushButton, closePB, clicked, bool,
				QSearch, form_, slotClose, void);

    Q_CONNECT_1(QPushButton, findPB, clicked, bool,
				QSearchDialog, this, findClicked, void) ;

    Q_CONNECT_1(QPushButton, replacePB, clicked, bool,
				QSearchDialog, this, replaceClicked, void);

    Q_CONNECT_1(QPushButton, replaceallPB, clicked, bool, 
				QSearchDialog, this, replaceallClicked, void);

	Q_CONNECT_1(QComboBox, findCO, editTextChanged, const QString&, 
				QSearchDialog, this, findChanged, void);



	//check<const QString&>(findCO);
//	check(findCO, &QComboBox::editTextChanged);

	//check<void>(this);
}


void QSearchDialog::show()
{
	QDialog::show();
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
