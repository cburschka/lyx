/**
 * \file QBoxDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QBox.h"
#include "QBoxDialog.h"
#include "qt_helpers.h"

#include "lengthcombo.h"
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

namespace lyx {
namespace frontend {

QBoxDialog::QBoxDialog(QBox * form)
	: QBoxDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QBoxDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QBoxDialog::change_adaptor()
{
	form_->changed();
}


void QBoxDialog::innerBoxChanged(const QString & str)
{
	bool ibox = (str != qt_("None"));
	valignCO->setEnabled(ibox);
	ialignCO->setEnabled(ibox);
	halignCO->setEnabled(!ibox);
	heightED->setEnabled(ibox);
	heightUnitsLC->setEnabled(ibox);
	form_->setSpecial(ibox);
}


void QBoxDialog::typeChanged(int index)
{
	bool frameless = (index == 0);
	if (frameless) {
		valignCO->setEnabled(true);
		ialignCO->setEnabled(true);
		halignCO->setEnabled(false);
		heightED->setEnabled(true);
		heightUnitsLC->setEnabled(true);
		form_->setSpecial(true);
	}
	int itype = innerBoxCO->currentItem();
	form_->setInnerType(frameless, itype);
}


void QBoxDialog::restoreClicked()
{
	form_->setInnerType(true, 2);
	widthED->setText("100");
	widthUnitsLC->setCurrentItem(LyXLength::PCW);
	heightED->setText("1");
	for (int j = 0; j < heightUnitsLC->count(); j++) {
		if (heightUnitsLC->text(j) == qt_("Total Height"))
			heightUnitsLC->setCurrentItem(j);
	}
}

} // namespace frontend
} // namespace lyx
