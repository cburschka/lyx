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

#include "ControlSpellchecker.h"
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

GuiSpellcheckerDialog::GuiSpellcheckerDialog(LyXView & lv)
	: GuiDialog(lv, "spellchecker")
{
	setupUi(this);
	setViewTitle(_("Spellchecker"));
	setController(new ControlSpellchecker(*this));

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

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

	wordED->setReadOnly(true);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
}


ControlSpellchecker & GuiSpellcheckerDialog::controller()
{
	return static_cast<ControlSpellchecker &>(GuiDialog::controller());
}


void GuiSpellcheckerDialog::acceptClicked()
{
	accept();
}


void GuiSpellcheckerDialog::addClicked()
{
	add();
}


void GuiSpellcheckerDialog::replaceClicked()
{
	replace();
}


void GuiSpellcheckerDialog::ignoreClicked()
{
	ignore();
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
	slotClose();
	GuiDialog::closeEvent(e);
}


void GuiSpellcheckerDialog::reject()
{
	slotClose();
	QDialog::reject();
}


void GuiSpellcheckerDialog::updateContents()
{
	if (isVisibleView() || controller().exitEarly())
		controller().check();
}


void GuiSpellcheckerDialog::accept()
{
	controller().ignoreAll();
}


void GuiSpellcheckerDialog::add()
{
	controller().insert();
}


void GuiSpellcheckerDialog::ignore()
{
	controller().check();
}


void GuiSpellcheckerDialog::replace()
{
	controller().replace(qstring_to_ucs4(replaceCO->currentText()));
}


void GuiSpellcheckerDialog::partialUpdate(int state)
{
	switch (state) {
		case ControlSpellchecker::SPELL_PROGRESSED:
			spellcheckPR->setValue(controller().getProgress());
			break;

		case ControlSpellchecker::SPELL_FOUND_WORD: {
			wordED->setText(toqstr(controller().getWord()));
			suggestionsLW->clear();

			docstring w;
			while (!(w = controller().getSuggestion()).empty())
				suggestionsLW->addItem(toqstr(w));

			if (suggestionsLW->count() == 0)
				suggestionChanged(new QListWidgetItem(wordED->text()));
			else
				suggestionChanged(suggestionsLW->item(0));

			suggestionsLW->setCurrentRow(0);
			break;
		}
	}
}

} // namespace frontend
} // namespace lyx

#include "GuiSpellchecker_moc.cpp"
