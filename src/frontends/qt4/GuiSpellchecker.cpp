/**
 * \file GuiSpellchecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSpellchecker.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

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
// GuiSpellCheckerDialog
//
/////////////////////////////////////////////////////////////////////


GuiSpellcheckerDialog::GuiSpellcheckerDialog(GuiSpellchecker * form)
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


void GuiSpellcheckerDialog::acceptClicked()
{
	form_->accept();
}

void GuiSpellcheckerDialog::addClicked()
{
	form_->add();
}

void GuiSpellcheckerDialog::replaceClicked()
{
	form_->replace();
}

void GuiSpellcheckerDialog::ignoreClicked()
{
	form_->ignore();
}

void GuiSpellcheckerDialog::suggestionChanged(QListWidgetItem * item)
{
	if (replaceCO->count() != 0)
		replaceCO->setItemText(0, item->text());
	else
		replaceCO->addItem(item->text());

	replaceCO->setCurrentIndex(0);
}

void GuiSpellcheckerDialog::replaceChanged(const QString & str)
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


void GuiSpellcheckerDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiSpellcheckerDialog::reject()
{
	form_->slotWMHide();
	QDialog::reject();
}



/////////////////////////////////////////////////////////////////////
//
// GuiSpellChecker
//
/////////////////////////////////////////////////////////////////////


GuiSpellchecker::GuiSpellchecker(Dialog & parent)
	: GuiView<GuiSpellcheckerDialog>(parent, _("Spellchecker"))
{}


void GuiSpellchecker::build_dialog()
{
	dialog_.reset(new GuiSpellcheckerDialog(this));

	bcview().setCancel(dialog_->closePB);
	dialog_->wordED->setReadOnly(true);
}


void GuiSpellchecker::update_contents()
{
	if (isVisible() || controller().exitEarly())
		controller().check();
}


void GuiSpellchecker::accept()
{
	controller().ignoreAll();
}


void GuiSpellchecker::add()
{
	controller().insert();
}


void GuiSpellchecker::ignore()
{
	controller().check();
}


void GuiSpellchecker::replace()
{
	controller().replace(qstring_to_ucs4(dialog_->replaceCO->currentText()));
}


void GuiSpellchecker::partialUpdate(int s)
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

#include "GuiSpellchecker_moc.cpp"
