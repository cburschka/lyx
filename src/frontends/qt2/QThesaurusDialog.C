/**
 * \file QThesaurusDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector>

#include "LString.h"

#include "ControlThesaurus.h"
#include "QThesaurusDialog.h"
#include "Dialogs.h"
#include "QThesaurus.h"

#include <qpushbutton.h>
#include <qlistview.h>
#include <qlineedit.h>

QThesaurusDialog::QThesaurusDialog(QThesaurus * form)
	: QThesaurusDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QThesaurusDialog::change_adaptor()
{
	form_->changed();
}


void QThesaurusDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QThesaurusDialog::entryChanged()
{
	updateLists();
}


void QThesaurusDialog::replaceClicked()
{
	form_->replace();
}


void QThesaurusDialog::selectionChanged(QListViewItem * item)
{
	if (form_->readOnly())
		return;

	string const entry(item->text(0).latin1());
	replaceED->setText(entry.c_str());
	replacePB->setEnabled(true);
	form_->changed();
}


void QThesaurusDialog::selectionClicked(QListViewItem * item)
{
	entryED->setText(item->text(0));
	selectionChanged(item);
	updateLists();
}


void QThesaurusDialog::updateLists()
{
	meaningsLV->clear();

	std::vector<string> matches;

	meaningsLV->setUpdatesEnabled(false);

	Thesaurus::Meanings meanings = form_->controller().getMeanings(entryED->text().latin1());

	for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
		cit != meanings.end(); ++cit) {
		QListViewItem * i = new QListViewItem(meaningsLV);
		i->setText(0, cit->first.c_str());
		i->setOpen(true);
		for (std::vector<string>::const_iterator cit2 = cit->second.begin();
			cit2 != cit->second.end(); ++cit2) {
				QListViewItem * i2 = new QListViewItem(i);
				i2->setText(0, cit2->c_str());
				i2->setOpen(true);
			}
	}

	meaningsLV->setUpdatesEnabled(true);
	meaningsLV->update();
}
