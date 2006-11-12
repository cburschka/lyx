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

#include "QBoxDialog.h"
//Added by qt3to4:
#include <QCloseEvent>

#include "lengthcombo.h"
#include "validators.h"
#include "QBox.h"
#include "qt_helpers.h"

#include <qlineedit.h>
#include <qpushbutton.h>

namespace lyx {
namespace frontend {

QBoxDialog::QBoxDialog(QBox * form)
	: form_(form)
{
	setupUi(this);
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( widthED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( widthUnitsLC, SIGNAL( selectionChanged(lyx::LyXLength::UNIT) ), this, SLOT( change_adaptor() ) );
    connect( valignCO, SIGNAL( highlighted(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( heightED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( heightUnitsLC, SIGNAL( selectionChanged(lyx::LyXLength::UNIT) ), this, SLOT( change_adaptor() ) );
    connect( restorePB, SIGNAL( clicked() ), this, SLOT( restoreClicked() ) );
    connect( typeCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( typeCO, SIGNAL( activated(int) ), this, SLOT( typeChanged(int) ) );
    connect( halignCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( ialignCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( innerBoxCO, SIGNAL( activated(const QString&) ), this, SLOT( innerBoxChanged(const QString &) ) );
    connect( innerBoxCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );

	heightED->setValidator(unsignedLengthValidator(heightED));
	widthED->setValidator(unsignedLengthValidator(widthED));
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
	int itype = innerBoxCO->currentIndex();
	form_->setInnerType(frameless, itype);
}


void QBoxDialog::restoreClicked()
{
	form_->setInnerType(true, 2);
	widthED->setText("100");
	widthUnitsLC->setCurrentItem(LyXLength::PCW);
	heightED->setText("1");
	for (int j = 0; j < heightUnitsLC->count(); j++) {
		if (heightUnitsLC->itemText(j) == qt_("Total Height"))
			heightUnitsLC->setCurrentItem(j);
	}
}

} // namespace frontend
} // namespace lyx

#include "QBoxDialog_moc.cpp"
