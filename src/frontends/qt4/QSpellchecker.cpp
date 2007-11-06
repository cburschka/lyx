/**
 * \file QSpellchecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QSpellchecker.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlSpellchecker.h"

#include <QProgressBar>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>


using std::string;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QSpellCheckerDialog
//
/////////////////////////////////////////////////////////////////////


QSpellcheckerDialog::QSpellcheckerDialog(QSpellchecker * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(replaceCO, SIGNAL(highlighted(const QString &)),
		this, SLOT(replaceChanged(const QString &)));
	connect(replacePB, SIGNAL(clicked()),
		this, SLOT(replaceClicked()));
	connect(ignorePB, SIGNAL(clicked()),
		this, SLOT(ignoreClicked()));
	connect(replacePB_3, SIGNAL(clicked()),
		this, SLOT(acceptClicked()));
	connect(addPB, SIGNAL(clicked()),
		this, SLOT(addClicked()));
	connect(suggestionsLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this, SLOT(replaceClicked() ) );
	connect(suggestionsLW, SIGNAL(itemClicked(QListWidgetItem*)),
		this, SLOT(suggestionChanged(QListWidgetItem*)));
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

void QSpellcheckerDialog::suggestionChanged(QListWidgetItem * item)
{
	if (replaceCO->count() != 0)
		replaceCO->setItemText(0, item->text());
	else
		replaceCO->addItem(item->text());

	replaceCO->setCurrentIndex(0);
}

void QSpellcheckerDialog::replaceChanged(const QString & str)
{
	if (suggestionsLW->currentItem()->text() == str)
		return;

	for (int i = 0; i < suggestionsLW->count(); ++i) {
		if (suggestionsLW->item(i)->text() == str) {
			suggestionsLW->setCurrentRow(i);
			break;
		}
	}
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



/////////////////////////////////////////////////////////////////////
//
// QSpellChecker
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlSpellchecker, QView<QSpellcheckerDialog> >
	SpellcheckerBase;

QSpellchecker::QSpellchecker(Dialog & parent)
	: SpellcheckerBase(parent, _("Spellchecker"))
{}


void QSpellchecker::build_dialog()
{
	dialog_.reset(new QSpellcheckerDialog(this));

	bcview().setCancel(dialog_->closePB);
	dialog_->wordED->setReadOnly(true);
}


void QSpellchecker::update_contents()
{
	// The clauses below are needed because the spellchecker
	// controller has many flaws (see bugs 1950, 2218).
	// Basically, we have to distinguish the case where a
	// spellcheck has already been performed for the whole
	// document (exitEarly() == true, isVisible() == false) 
	// from the rest (exitEarly() == false, isVisible() == true).
	static bool check_after_early_exit;
	if (controller().exitEarly()) {
		// a spellcheck has already been performed,
		controller().check();
		check_after_early_exit = true;
	}
	else if (isVisible()) {
		// the above check triggers a second update,
		// and isVisible() is true then. Prevent a
		// second check that skips the first word
		if (check_after_early_exit)
			// don't check, but reset the bool.
			// business as usual after this.
			check_after_early_exit = false;
		else
			// perform spellcheck (default case)
			controller().check();
	}
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
	controller().replace(qstring_to_ucs4(dialog_->replaceCO->currentText()));
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

		docstring w;
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

#include "QSpellchecker_moc.cpp"
