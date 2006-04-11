/**
 * \file QVSpaceDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QVSpaceDialog.h"
#include "QVSpace.h"
//Added by qt3to4:
#include <QCloseEvent>

#include "lengthcombo.h"
#include "validators.h"
#include "qt_helpers.h"

#include <QLineEdit>
#include <QPushButton>
#include <QValidator>


namespace lyx {
namespace frontend {


QVSpaceDialog::QVSpaceDialog(QVSpace * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));

    connect( spacingCO, SIGNAL( highlighted(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( valueLE, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( spacingCO, SIGNAL( activated(int) ), this, SLOT( enableCustom(int) ) );
    connect( keepCB, SIGNAL( clicked() ), this, SLOT( change_adaptor() ) );
    connect( unitCO, SIGNAL( selectionChanged(LyXLength::UNIT) ), this, SLOT( change_adaptor() ) );

	valueLE->setValidator(unsignedLengthValidator(valueLE));
}


void QVSpaceDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QVSpaceDialog::change_adaptor()
{
	form_->changed();
}


void QVSpaceDialog::enableCustom(int selection)
{
	bool const enable = selection == 5;
	valueLE->setEnabled(enable);
	unitCO->setEnabled(enable);
}

} // namespace frontend
} // namespace lyx
