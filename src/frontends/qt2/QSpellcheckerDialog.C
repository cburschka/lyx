/**
 * \file QSpellcheckerDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

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


void QSpellcheckerDialog:: suggestionChanged(const QString & str)
{
	if (replaceCO->count() != 0)
		replaceCO->changeItem(str, 0);
	else
		replaceCO->insertItem(str);

	replaceCO->setCurrentItem(0);
}


void QSpellcheckerDialog:: replaceChanged(const QString & str)
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
