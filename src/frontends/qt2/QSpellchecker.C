/**
 * \file QSpellchecker.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlSpellchecker.h"
#include "QSpellcheckerDialog.h"
#include "QSpellchecker.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "debug.h"

#include <qprogressbar.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcombobox.h>

typedef Qt2CB<ControlSpellchecker, Qt2DB<QSpellcheckerDialog> > base_class;


QSpellchecker::QSpellchecker()
	: base_class(qt_("Spellchecker"))
{
}


void QSpellchecker::build_dialog()
{
	dialog_.reset(new QSpellcheckerDialog(this));

	bc().setCancel(dialog_->closePB);
	dialog_->wordED->setReadOnly(true);
}


void QSpellchecker::update_contents()
{
	dialog_->wordED->setText("");
	dialog_->replaceCO->clear();
	dialog_->suggestionsLB->clear();
	dialog_->spellcheckPR->setProgress(0);
	dialog_->spellcheckPB->setEnabled(true);
	dialog_->wordED->setEnabled(false);
	dialog_->replaceCO->setEnabled(false);
	dialog_->replacePB->setEnabled(false);
	dialog_->ignorePB->setEnabled(false);
	dialog_->replacePB_3->setEnabled(false);
	dialog_->addPB->setEnabled(false);
}


void QSpellchecker::accept()
{
	controller().ignoreAll();
}


void QSpellchecker::add()
{
	controller().insert();
}


void QSpellchecker::ignore()
{
	controller().check();
}


void QSpellchecker::replace()
{
	controller().replace(fromqstr(dialog_->replaceCO->currentText()));
}


void QSpellchecker::spellcheck()
{
	dialog_->spellcheckPB->setEnabled(false);
	dialog_->wordED->setEnabled(true);
	dialog_->replaceCO->setEnabled(true);
	dialog_->replacePB->setEnabled(true);
	dialog_->ignorePB->setEnabled(true);
	dialog_->replacePB_3->setEnabled(true);
	dialog_->addPB->setEnabled(true);
	controller().check();
}


void QSpellchecker::stop()
{
	controller().stop();
	dialog_->spellcheckPB->setEnabled(true);
	hide();
}


void QSpellchecker::partialUpdate(int id)
{
	switch (id) {
	case 0:
		dialog_->spellcheckPR->setProgress(controller().getProgress());
		break;

	case 1: {
		dialog_->wordED->setText(toqstr(controller().getWord()));
		dialog_->suggestionsLB->clear();

		string w;
		while (!(w = controller().getSuggestion()).empty()) {
			dialog_->suggestionsLB->insertItem(toqstr(w));
		}

		if (dialog_->suggestionsLB->count() == 0) {
			dialog_->suggestionChanged(dialog_->wordED->text());
		} else {
			dialog_->suggestionChanged(dialog_->suggestionsLB->text(0));
		}
	}
		break;

	case 2:
		dialog_->spellcheckPB->setEnabled(true);
		hide();
		QMessageBox::information(0, qt_("Spellcheck complete"),
					 toqstr(controller().getMessage()),
					 qt_("OK"));
		break;
	}
}
