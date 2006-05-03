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

#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlSpellchecker.h"

#include <QProgressBar>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlSpellchecker, QView<QSpellcheckerDialog> > base_class;

QSpellchecker::QSpellchecker(Dialog & parent)
	: base_class(parent, _("Spellchecker"))
{}


void QSpellchecker::build_dialog()
{
	dialog_.reset(new QSpellcheckerDialog(this));

	bcview().setCancel(dialog_->closePB);
	dialog_->wordED->setReadOnly(true);
}


void QSpellchecker::update_contents()
{
	controller().check();
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
		dialog_->spellcheckPR->setValue(controller().getProgress());
		break;

	case ControlSpellchecker::SPELL_FOUND_WORD: {
		dialog_->wordED->setText(toqstr(controller().getWord()));
		dialog_->suggestionsLW->clear();

		string w;
		while (!(w = controller().getSuggestion()).empty()) {
			dialog_->suggestionsLW->addItem(toqstr(w));
		}

		if (dialog_->suggestionsLW->count() == 0) {
			dialog_->suggestionChanged(new QListWidgetItem(dialog_->wordED->text()));
		} else {
			dialog_->suggestionChanged(dialog_->suggestionsLW->item(0));
		}

		dialog_->suggestionsLW->setCurrentRow(0);
	}
		break;

	}
}

} // namespace frontend
} // namespace lyx
