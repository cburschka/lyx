/**
 * \file QCharacterDialog.C
 * Copyright 2001 the LyX Team
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include "QCharacterDialog.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>

QCharacterDialog::QCharacterDialog(QCharacter * form)
	: QCharacterDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


void QCharacterDialog::change_adaptor()
{
	form_->changed();

	if (!autoapplyCB->isChecked())
		return;

	// to be really good here, we should set the combos to the values of
	// the current text, and make it appear as "no change" if the values
	// stay the same between applys. Might be difficult though wrt to a
	// moved cursor - jbl
	form_->slotApply();
	familyCO->setCurrentItem(0);
	seriesCO->setCurrentItem(0);
	sizeCO->setCurrentItem(0);
	shapeCO->setCurrentItem(0);
	miscCO->setCurrentItem(0);
	langCO->setCurrentItem(0);
	colorCO->setCurrentItem(0);
}


void QCharacterDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
