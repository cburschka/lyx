/**
 * \file QSpellchecker.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
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
	: base_class(_("LyX: Spell-check Document"))
{
}


void QSpellchecker::build_dialog()
{
	dialog_.reset(new QSpellcheckerDialog(this));

	bcview().setCancel(dialog_->closePB);
	dialog_->wordED->setReadOnly(true);
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


void QSpellchecker::partialUpdate(int s)
{
	ControlSpellchecker::State const state =
		static_cast<ControlSpellchecker::State>(s);

	switch (state) {

	case ControlSpellchecker::SPELL_PROGRESSED:
		dialog_->spellcheckPR->setProgress(controller().getProgress());
		break;

	case ControlSpellchecker::SPELL_FOUND_WORD: {
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

	}
}
