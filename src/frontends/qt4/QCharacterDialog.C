/**
 * \file QCharacterDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QCharacterDialog.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QCharacterDialog::QCharacterDialog(QCharacter * form)
	: form_(form)
{
	setupUi(this);
	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));

    connect( miscCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( sizeCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( familyCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( seriesCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( shapeCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( colorCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( langCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( toggleallCB, SIGNAL( clicked() ), this, SLOT( change_adaptor() ) );
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
	familyCO->setCurrentIndex(0);
	seriesCO->setCurrentIndex(0);
	sizeCO->setCurrentIndex(0);
	shapeCO->setCurrentIndex(0);
	miscCO->setCurrentIndex(0);
	langCO->setCurrentIndex(0);
	colorCO->setCurrentIndex(0);
}


void QCharacterDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QCharacterDialog_moc.cpp"
