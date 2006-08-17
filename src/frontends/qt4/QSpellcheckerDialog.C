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

#include <QListWidget>
#include <QPushButton>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QSpellcheckerDialog::QSpellcheckerDialog(QSpellchecker * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	connect( replaceCO, SIGNAL( highlighted(const QString&) ),
		this, SLOT( replaceChanged(const QString &) ) );
	connect( replacePB, SIGNAL( clicked() ),
		this, SLOT( replaceClicked() ) );
	connect( ignorePB, SIGNAL( clicked() ),
		this, SLOT( ignoreClicked() ) );
	connect( replacePB_3, SIGNAL( clicked() ),
		this, SLOT( acceptClicked() ) );
	connect( addPB, SIGNAL( clicked() ),
		this, SLOT( addClicked() ) );
	connect( suggestionsLW, SIGNAL( itemDoubleClicked(QListWidgetItem*) ),
		this, SLOT( replaceClicked() ) );
	connect( suggestionsLW, SIGNAL( itemClicked(QListWidgetItem*) ),
		this, SLOT( suggestionChanged(QListWidgetItem*) ) );
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

} // namespace frontend
} // namespace lyx

#include "QSpellcheckerDialog_moc.cpp"
