/**
 * \file QSpellcheckerDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QSpellcheckerDialog.h"
#include "QSpellchecker.h"

#include <Q3ListBox>
#include <QPushButton>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QSpellcheckerDialog::QSpellcheckerDialog(QSpellchecker * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
    
    connect( replaceCO, SIGNAL( highlighted(const QString&) ), this, SLOT( replaceChanged(const QString &) ) );
    connect( replacePB, SIGNAL( clicked() ), this, SLOT( replaceClicked() ) );
    connect( ignorePB, SIGNAL( clicked() ), this, SLOT( ignoreClicked() ) );
    connect( replacePB_3, SIGNAL( clicked() ), this, SLOT( acceptClicked() ) );
    connect( addPB, SIGNAL( clicked() ), this, SLOT( addClicked() ) );
    connect( suggestionsLB, SIGNAL( doubleClicked(QListBoxItem*) ), this, SLOT( replaceClicked() ) );
    connect( suggestionsLB, SIGNAL( highlighted(const QString&) ), this, SLOT( suggestionChanged(const QString &) ) );
}


void QSpellcheckerDialog::acceptClicked()
{
	form_->accept();
}

void QSpellcheckerDialog::addClicked()
{
	form_->add();
}

void QSpellcheckerDialog::replaceClicked()
{
	form_->replace();
}

void QSpellcheckerDialog::ignoreClicked()
{
	form_->ignore();
}

void QSpellcheckerDialog::suggestionChanged(const QString & str)
{
	if (replaceCO->count() != 0)
		replaceCO->changeItem(str, 0);
	else
		replaceCO->insertItem(str);

	replaceCO->setCurrentItem(0);
}

void QSpellcheckerDialog::replaceChanged(const QString & str)
{
	if (suggestionsLB->currentText() == str)
		return;

	unsigned int i = 0;
	for (; i < suggestionsLB->count(); ++i) {
		if (suggestionsLB->text(i) == str)
			break;
	}

	if (i != suggestionsLB->count())
		suggestionsLB->setCurrentItem(i);
}


void QSpellcheckerDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QSpellcheckerDialog::reject()
{
	form_->slotWMHide();
	QDialog::reject();
}

} // namespace frontend
} // namespace lyx
