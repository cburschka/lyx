/**
 * \file QSpellcheckerDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QSpellcheckerDialog.h"
#include "QSpellchecker.h"
#include "gettext.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qcombobox.h>


QSpellcheckerDialog::QSpellcheckerDialog(QSpellchecker * form)
	: QSpellcheckerDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		this, SLOT(stop()));
}


void QSpellcheckerDialog::stop()
{
	form_->stop();
}


void QSpellcheckerDialog::acceptClicked()
{
	form_->accept();
}


void QSpellcheckerDialog::spellcheckClicked()
{
	form_->spellcheck();
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


void QSpellcheckerDialog::suggestionChanged(QString const & str)
{
	if (replaceCO->count() != 0)
		replaceCO->changeItem(str, 0);
	else
		replaceCO->insertItem(str);

	replaceCO->setCurrentItem(0);
}


void QSpellcheckerDialog::replaceChanged(QString const & str)
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
