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

#include "lengthcombo.h"
#include "qt_helpers.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qvalidator.h>


namespace lyx {
namespace frontend {

QVSpaceDialog::QVSpaceDialog(QVSpace * form)
	: QVSpaceDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
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


void QVSpaceDialog::enableCustom(int)
{
	bool const enable = spacingCO->currentItem()==5;
	valueLE->setEnabled(enable);
	unitCO->setEnabled(enable);
}

} // namespace frontend
} // namespace lyx
