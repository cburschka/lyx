/**
 * \file QSpellchecker.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "ControlSpellchecker.h"
#include "QSpellcheckerDialog.h"
#include "QSpellchecker.h"
#include "Qt2BC.h"
#include "gettext.h"

#include <qprogressbar.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcombobox.h>

typedef Qt2CB<ControlSpellchecker, Qt2DB<QSpellcheckerDialog> > base_class;

QSpellchecker::QSpellchecker(ControlSpellchecker & c, Dialogs &)
	: base_class(c, _("Spellchecker"))
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
	controller().replace(dialog_->replaceCO->currentText().latin1());
}


void QSpellchecker::options()
{
	controller().options();
}


void QSpellchecker::spellcheck()
{
	controller().check();
	dialog_->spellcheckPB->setEnabled(false);
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
	case 1:
	{
		dialog_->wordED->setText(controller().getWord().c_str());
		dialog_->suggestionsLB->clear();

		string w;
		while (!(w = controller().getSuggestion()).empty()) {
			dialog_->suggestionsLB->insertItem(w.c_str());
		}
	}
		break;
	case 2:
		dialog_->spellcheckPB->setEnabled(true);
		hide();
		QMessageBox::information(0, _("Spellcheck complete"), controller().getMessage().c_str() , _("OK"));
		break;
	}
}
